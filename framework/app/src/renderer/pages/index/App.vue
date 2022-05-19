<script setup lang="ts">
import { getCurrentInstance, onBeforeUnmount, onMounted } from 'vue';
import KfSystemPrepareModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSystemPrepareModal.vue';
import KfLayoutVue from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfLayout.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import zhCN from 'ant-design-vue/es/locale/zh_CN';
import {
  markClearDB,
  markClearJournal,
  removeLoadingMask,
  useIpcListener,
  handleOpenLogviewByFile,
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
import { bindIPCListener } from '@kungfu-trader/kungfu-app/src/renderer/ipcMsg/ipcListener';
import { useTradingTask } from '@kungfu-trader/kungfu-app/src/components/modules/tradingTask/utils';
import {
  getAllRiskSettingList,
  setAllRiskSettingList,
} from '@kungfu-trader/kungfu-js-api/actions';

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
      case 'open-log':
        handleOpenLogviewByFile();
        break;
      case 'export-all-trading-data':
        globalBus.next({
          tag: 'export',
          tradingDataType: 'all',
        } as ExportTradingDataEvent);
    }
  }
  if (data.tag === 'update:riskSetting') {
    setAllRiskSettingList(data.riskSettingList);
    store.setRiskSettingList(data.riskSettingList);
  }
});

const {
  setTradingTaskModalVisible,
  currentSelectedTradingTaskExtKey,
  setTradingTaskConfigPayload,
  handleConfirmAddUpdateTask,
} = useTradingTask();

onMounted(() => {
  bindIPCListener(store);
  removeLoadingMask();

  getAllRiskSettingList().then((res: KungfuApi.RiskSetting[]) => {
    store.setRiskSettingList(res);
  });

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
      :title="$t('system_prompt')"
      :visible="preStartSystemLoading"
      :status="[
        { key: 'archive', status: preStartSystemLoadingData.archive },
        { key: 'watcher', status: preStartSystemLoadingData.watcher },
        {
          key: 'systemLoading',
          status: preStartSystemLoadingData.systemLoading,
        },
      ]"
      :txt="{
        archive: { done: $t('archive_completion'), loading: $t('archive') },
        watcher: {
          done: $t('environment_complete'),
          loading: $t('environment_preparation'),
        },
        systemLoading: {
          done: $t('ready'),
          loading: $t('wait_ready'),
        },
      }"
    ></KfSystemPrepareModal>
    <KfSystemPrepareModal
      :title="$t('system_prompt')"
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
        record: { done: $t('saving_completed'), loading: $t('save_data') },
        quit: {
          done: $t('end_all_transactions'),
          loading: $t('closing'),
        },
      }"
    ></KfSystemPrepareModal>

    <!-- global modal start -->

    <!-- export trading data -->
    <KfDownloadDateModal
      v-if="exportDateModalVisible"
      v-model:visible="exportDateModalVisible"
      @confirm="handleConfirmExportDate"
    ></KfDownloadDateModal>

    <!-- add/update trading task -->
    <KfSetByConfigModal
      v-if="setTradingTaskModalVisible"
      v-model:visible="setTradingTaskModalVisible"
      :payload="setTradingTaskConfigPayload"
      :primaryKeyUnderline="true"
      @confirm="
        handleConfirmAddUpdateTask($event, currentSelectedTradingTaskExtKey)
      "
    ></KfSetByConfigModal>

    <!-- global modal end -->

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
