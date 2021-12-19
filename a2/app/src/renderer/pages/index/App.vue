<script setup lang="ts">
import { computed, getCurrentInstance, onMounted, reactive, ref } from 'vue';
import KfSystemPrepareModal from '@renderer/components/public/KfSystemPrepareModal.vue';

import zhCN from 'ant-design-vue/es/locale/zh_CN';
import {
    markClearJournal,
    removeLoadingMask,
    useIpcListener,
} from '@renderer/assets/methods/uiUtils';
import bus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import { preQuitTasks } from '@renderer/assets/methods/actionsUtils';
import { ipcRenderer } from 'electron';
import {
    dealAppStates,
    dealAssetsByHolderUID,
    watcher,
} from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { tradingDataSubject } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import { useGlobalStore } from './store/global';
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
                preQuitTasks().finally(() => {
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
store.setKfConfigList();
store.setKfExtConfigs();
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
    </a-config-provider>
</template>

<style lang="less">
@import '@renderer/assets/less/base.less';
@import '@renderer/assets/less/public.less';
@import '@renderer/assets/less/coverAnt.less';

#app {
    width: 100%;
    height: 100%;
    font-family: Avenir, Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;

    .app__warp {
        height: 100%;
        width: 100%;
    }
}
</style>
