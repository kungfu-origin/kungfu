<script setup lang="ts">
import { computed, getCurrentInstance, onMounted, reactive, ref } from 'vue';
import KfSystemPrepareModal from '@renderer/components/public/KfSystemPrepareModal.vue';

import zhCN from 'ant-design-vue/es/locale/zh_CN';
import {
    getIfSaveInstruments,
    markClearJournal,
    removeLoadingMask,
    useIpcListener,
} from '@renderer/assets/methods/uiUtils';
import bus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import {
    preQuitTasks,
    useDealExportHistoryTradingData,
} from '@renderer/assets/methods/actionsUtils';
import { ipcRenderer } from 'electron';
import {
    dealAppStates,
    dealAssetsByHolderUID,
    watcher,
} from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { tradingDataSubject } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import { useGlobalStore } from './store/global';
import KfDownloadDateModal from '@renderer/components/layout/KfHistoryDateModal.vue';
import { throttleTime } from 'rxjs';
import { kfRequestMarketData } from '@kungfu-trader/kungfu-js-api/kungfu';
import workers from '@renderer/assets/workers';
import dayjs from 'dayjs';

const app = getCurrentInstance();

const preStartSystemLoadingData = reactive<Record<string, 'loading' | 'done'>>({
    archive: 'loading',
    watcher: 'loading',
});

const preQuitSystemLoadingData = reactive<
    Record<string, 'loading' | 'done' | undefined>
>({
    record: undefined,
    quit: undefined,
});

const preStartSystemLoading = computed(() => {
    return (
        Object.values(preStartSystemLoadingData).filter(
            (item: string) => item !== 'done',
        ).length > 0
    );
});

const preQuitSystemLoading = computed(() => {
    return (
        Object.values(preQuitSystemLoadingData).filter(
            (item: string | undefined) => item !== undefined,
        ).length > 0
    );
});

const { exportDateModalVisible, exportDataLoading, handleConfirmExportDate } =
    useDealExportHistoryTradingData();

onMounted(() => {
    removeLoadingMask();

    window.addEventListener('resize', () => {
        app?.proxy &&
            app?.proxy.$bus.next({
                tag: 'resize',
            } as ResizeEvent);
    });
});

const timer = setInterval(() => {
    if (watcher?.isLive()) {
        preStartSystemLoadingData.watcher = 'done';
        clearInterval(timer);
    } else {
        preStartSystemLoadingData.watcher = 'loading';
    }
}, 500);

useIpcListener();
bus.subscribe((data: KfBusEvent) => {
    if (data.tag === 'processStatus') {
        if (data.name && data.name === 'archive') {
            preStartSystemLoadingData.archive =
                data.status === 'online' ? 'loading' : 'done';
        }
    }

    if (data.tag === 'main') {
        switch (data.name) {
            case 'clear-journal':
                markClearJournal();
                break;
            case 'record-before-quit':
                preQuitSystemLoadingData.record = 'loading';
                preQuitTasks([saveBoardsMap()]).finally(() => {
                    ipcRenderer.send('record-before-quit-done');
                    preQuitSystemLoadingData.record = 'done';
                });
                break;
            case 'clear-process-before-quit-start':
                preQuitSystemLoadingData.quit = 'loading';
                break;
            case 'clear-process-before-quit-end':
                preQuitSystemLoadingData.quit = 'done';
                break;
            case 'export-all-trading-data':
                bus.next({
                    tag: 'export',
                    tradingDataType: 'all',
                } as ExportTradingDataEvent);
        }
    }
});

const store = useGlobalStore();
tradingDataSubject.subscribe((watcher: Watcher) => {
    const appStates = dealAppStates(watcher.appStates);
    store.setAppStates(appStates);
    const assets = dealAssetsByHolderUID(watcher.ledger.Asset);
    store.setAssets(assets);
});

const subscribedInstruments: Record<string, boolean> = {};
tradingDataSubject.pipe(throttleTime(3000)).subscribe((watcher: Watcher) => {
    const bigint0 = BigInt(0);
    const positions = watcher.ledger.Position.filter('ledger_category', 0)
        .nofilter('volume', bigint0)
        .list()
        .map(
            (item: Position): InstrumentForSub => ({
                uidKey: item.uid_key,
                exchangeId: item.exchange_id,
                instrumentId: item.instrument_id,
                instrumentType: item.instrument_type,
                mdLocation: watcher.getLocation(item.holder_uid),
            }),
        );

    positions.forEach((item) => {
        if (subscribedInstruments[item.uidKey]) {
            return;
        }
        kfRequestMarketData(
            watcher,
            item.exchangeId,
            item.instrumentId,
            item.mdLocation,
        );
        subscribedInstruments[item.uidKey] = true;
    });
});

const dealInstrumentController = ref<boolean>(false);
const oldInstruments: InstrumentResolved[] = JSON.parse(
    localStorage.getItem('instruments') || '[]',
);
const oldInstrumentsLength = ref<number>(oldInstruments.length || 0);
tradingDataSubject.pipe(throttleTime(5000)).subscribe((watcher: Watcher) => {
    const instruments = watcher.ledger.Instrument.list();
    if (!instruments || !instruments.length) {
        localStorage.setItem('instrumentsSavedDate', '');
        return;
    }

    if (
        getIfSaveInstruments(instruments, oldInstrumentsLength.value) &&
        !dealInstrumentController.value
    ) {
        dealInstrumentController.value = true;
        console.time('DealInstruments');
        console.log('DealInstruments postMessage', instruments.length);
        instruments.forEach((item: Instrument) => {
            item.ukey = item.uid_key;
        });
        workers.dealInstruments.postMessage({
            instruments: instruments,
        });
    }
});

if (oldInstrumentsLength.value) {
    store.setInstruments(oldInstruments);
}

workers.dealInstruments.onmessage = (event: {
    data: { instruments: InstrumentResolved[] };
}) => {
    const { instruments } = event.data || {};
    console.timeEnd('DealInstruments');
    console.log('DealInstruments onmessage', instruments.length);
    localStorage.setItem('instrumentsSavedDate', dayjs().format('YYYY-MM-DD'));
    localStorage.setItem('instruments', JSON.stringify(instruments || []));
    oldInstrumentsLength.value = instruments.length || 0; //refresh old instruments
    dealInstrumentController.value = false;
    if (instruments.length) {
        console.log(instruments[0], '---');
        store.setInstruments(instruments);
    }
};

store.setKfConfigList();
store.setKfExtConfigs();
store.setSubscribedInstruments();

function saveBoardsMap(): Promise<void> {
    if (app?.proxy) {
        const { boardsMap } = app?.proxy.$useGlobalStore();
        localStorage.setItem('boardsMap', JSON.stringify(boardsMap));
        return Promise.resolve();
    }
    return Promise.resolve();
}
</script>

<template>
    <a-config-provider :locale="zhCN">
        <div class="app__warp">
            <router-view />
        </div>
        <KfSystemPrepareModal
            title="系统提示"
            :visible="preStartSystemLoading"
            :status="[
                { key: 'archive', status: preStartSystemLoadingData.archive },
                { key: 'watcher', status: preStartSystemLoadingData.watcher },
            ]"
            :txt="{
                archive: { done: '功夫归档完成 ✓', loading: '功夫归档中...' },
                watcher: {
                    done: '功夫环境准备完成 ✓',
                    loading: '功夫环境准备中...',
                },
            }"
        ></KfSystemPrepareModal>
        <KfSystemPrepareModal
            title="系统提示"
            :visible="preQuitSystemLoading"
            :status="[
                {
                    key: 'record',
                    status: preQuitSystemLoadingData.record,
                },
                {
                    key: 'quit',
                    status: preQuitSystemLoadingData.quit,
                },
            ]"
            :txt="{
                record: { done: '保存数据完成 ✓', loading: '保存数据中...' },
                quit: {
                    done: '结束所有交易进程 ✓',
                    loading: '结束交易进程中，请勿关闭...',
                },
            }"
        ></KfSystemPrepareModal>
        <KfDownloadDateModal
            v-if="exportDateModalVisible"
            v-model:visible="exportDateModalVisible"
            @confirm="handleConfirmExportDate"
        ></KfDownloadDateModal>
        <a-spin v-if="exportDataLoading" :spinning="exportDataLoading"></a-spin>
    </a-config-provider>
</template>

<style lang="less">
@import '@renderer/assets/less/coverAnt.less';
@import '@renderer/assets/less/base.less';
@import '@renderer/assets/less/public.less';

#app {
    width: 100%;
    height: 100%;
    font-family: Consolas, Monaco, Lucida Console, Liberation Mono,
        DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;

    .app__warp {
        height: 100%;
        width: 100%;
    }
}
</style>
