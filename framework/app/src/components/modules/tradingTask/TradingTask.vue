<script lang="ts" setup>
import {
  useDashboardBodySize,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@renderer/assets/methods/uiUtils';
import { ref } from 'vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfSetExtensionModal from '@renderer/components/public/KfSetExtensionModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';

import { columns } from './config';
import { message } from 'ant-design-vue';
import path from 'path';
import {
  dealKfConfigValueByType,
  getPrimaryKeyFromKfConfigItem,
  getProcessIdByKfLocation,
  graceKillProcess,
  switchKfLocation,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  deleteProcess,
  startTask,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  removeKfLocation,
  removeLog,
} from '@kungfu-trader/kungfu-js-api/actions';

const { extConfigs } = useExtConfigsRelated();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { processStatusData } = useProcessStatusDetailData();

const setExtensionModalVisible = ref<boolean>(false);
const setTaskModalVisible = ref<boolean>(false);
const currentSelectedExtKey = ref<string>('');
const setTaskConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: '交易任务',
  config: {} as KungfuApi.KfExtConfig,
});

function handleOpenSetTaskDialog() {
  setExtensionModalVisible.value = true;
}

function handleOpenSetTaskModal(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedExtKey: string,
  taskConfig?: KungfuApi.KfConfig,
) {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    selectedExtKey
  ];

  currentSelectedExtKey.value = selectedExtKey;
  setTaskConfigPayload.value.type = type;
  setTaskConfigPayload.value.title = `${extConfig.name}`;
  setTaskConfigPayload.value.config = extConfig;
  setTaskConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (taskConfig) {
      setTaskConfigPayload.value.initValue = JSON.parse(taskConfig.value);
    }
  }

  if (!extConfig?.settings?.length) {
    message.error(
      `配置项不存在, 请检查 ${extConfig?.name || selectedExtKey} package.json`,
    );
    return;
  }

  setTaskModalVisible.value = true;
}

function handleConfirmAddUpdateTask(
  data: {
    formState: Record<string, KungfuApi.KfConfigValue>;
    idByPrimaryKeys: string;
    changeType: KungfuApi.ModalChangeType;
  },
  extKey: string,
) {
  const { formState } = data;
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    extKey
  ];
  const args: string = buildArgs(extConfig.settings, formState);

  const taskLocation: KungfuApi.KfLocation = {
    category: 'strategy',
    group: extKey,
    name: getTaskPrimaryKey(
      formState,
      getPrimaryKeyFromKfConfigItem(extConfig.settings),
    ),
    mode: 'LIVE',
  };

  if (!extConfig.extPath) {
    message.error(`配置项不存在, 请检查 ${extConfig?.name} .so`);
    return;
  }

  const soPath = path.join(extConfig.extPath, extKey);
  return preStartTask(taskLocation)
    .then(() => startTask(taskLocation, soPath, args))
    .catch((err: Error) => message.error(err.message));
}

function getTaskPrimaryKey(
  formState: Record<string, KungfuApi.KfConfigValue>,
  configItems: KungfuApi.KfConfigItem[],
) {
  return configItems
    .filter((item) => {
      return formState[item.key] !== undefined;
    })
    .map((item) => dealKfConfigValueByType(item.type, formState[item.key]))
    .join('-');
}

function preStartTask(taskLocation: KungfuApi.KfLocation) {
  return graceKillProcess(window.watcher, taskLocation, processStatusData.value)
    .then(() => removeKfLocation(taskLocation))
    .then(() => removeLog(taskLocation));
}

function buildArgs(
  settings: KungfuApi.KfConfigItem[],
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  return settings
    .filter((item) => {
      return formState[item.key] !== undefined;
    })
    .map((item) => {
      if (item.type === 'instrument') {
        const instrumentResolved = transformSearchInstrumentResultToInstrument(
          formState[item.key],
        );
        const { exchangeId, instrumentId } = instrumentResolved;
        return `${item.key}=${exchangeId}_${instrumentId}`;
      }
      return `${item.key}=${formState[item.key]}`;
    })
    .join(path.delimiter);
}
</script>

<template>
  <div class="kf-task-strategy__wrap kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <!-- <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            placeholder="关键字"
            style="width: 120px"
          />
        </KfDashboardItem> -->
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetTaskDialog"
          >
            添加
          </a-button>
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="[]"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :defaultExpandAllRows="true"
        emptyText="暂无数据"
      ></a-table>
    </KfDashboard>
    <KfSetExtensionModal
      v-if="setExtensionModalVisible"
      v-model:visible="setExtensionModalVisible"
      extensionType="strategy"
      @confirm="handleOpenSetTaskModal('add', $event)"
    ></KfSetExtensionModal>
    <KfSetByConfigModal
      v-if="setTaskModalVisible"
      v-model:visible="setTaskModalVisible"
      :payload="setTaskConfigPayload"
      :primaryKeyAvoidRepeatCompareTarget="[]"
      :primaryKeyAvoidRepeatCompareExtra="currentSelectedExtKey"
      @confirm="handleConfirmAddUpdateTask($event, currentSelectedExtKey)"
    ></KfSetByConfigModal>
  </div>
</template>
<style lang="less">
.kf-task-strategy__wrap {
  height: 100%;
}
</style>
