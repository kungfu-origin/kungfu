<script setup lang="ts">
import { getCurrentInstance, onBeforeUnmount, onMounted } from 'vue';
import KfSystemPrepareModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSystemPrepareModal.vue';
import KfLayoutVue from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfLayout.vue';

import zhCN from 'ant-design-vue/es/locale/zh_CN';
import {
  markClearDB,
  markClearJournal,
  removeLoadingMask,
  useIpcListener,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  useDealExportHistoryTradingData,
  useDealInstruments,
  usePreStartAndQuitApp,
  useSubscibeInstrumentAtEntry,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';

import { useGlobalStore } from './store/global';
import KfDownloadDateModal from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfHistoryDateModal.vue';
import { tradingDataSubject } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import globalBus from '../../assets/methods/globalBus';
import {
  dealAppStates,
  dealAssetsByHolderUID,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { bindIPCListener } from '@kungfu-trader/kungfu-app/src/renderer/ipcMsg/ipcListener'

const app = getCurrentInstance();
const store = useGlobalStore();

const {
  preStartSystemLoadingData,
  preStartSystemLoading,
  preQuitSystemLoadingData,
  preQuitSystemLoading,
} = usePreStartAndQuitApp();

useDealInstruments();
useSubscibeInstrumentAtEntry();

const { exportDateModalVisible, exportDataLoading, handleConfirmExportDate } =
  useDealExportHistoryTradingData();

useIpcListener();

const tradingDataSubscription = tradingDataSubject.subscribe(
  (watcher: KungfuApi.Watcher) => {
    const appStates = dealAppStates(watcher, watcher.appStates);
    store.setAppStates(appStates);
    const assets = dealAssetsByHolderUID(watcher, watcher.ledger.Asset);
    store.setAssets(assets);
  },
);

store.setKfConfigList();
store.setKfExtConfigs();
store.setSubscribedInstruments();

const busSubscription = globalBus.subscribe((data: KfBusEvent) => {
  if (data.tag === 'main') {
    switch (data.name) {
      case 'clear-journal':
        markClearJournal();
        break;
      case 'clear-db':
        markClearDB();
        break;
      case 'export-all-trading-data':
        globalBus.next({
          tag: 'export',
          tradingDataType: 'all',
        } as ExportTradingDataEvent);
    }
  }
});

onMounted(() => {
    bindIPCListener(store)
  removeLoadingMask();

  window.addEventListener('resize', () => {
    app?.proxy &&
      app?.proxy.$globalBus.next({
        tag: 'resize',
      } as ResizeEvent);
  });
});

onBeforeUnmount(() => {
  tradingDataSubscription.unsubscribe();
  busSubscription.unsubscribe();
});
</script>

<template>
  <a-config-provider :locale="zhCN" :autoInsertSpaceInButton="false">
    <div class="app__warp">
      <KfLayoutVue>
        <router-view />
      </KfLayoutVue>
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
          loading: '结束交易进程中, 请勿关闭...',
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
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/coverAnt.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/base.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/public.less';

#app {
  width: 100%;
  height: 100%;
  font-family: Consolas, Monaco, Lucida Console, Liberation Mono,
    DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  .app__warp {
    height: 100%;
    width: 100%;
  }
}
</style>
