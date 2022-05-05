<script lang="ts" setup>
import {
  confirmModal,
  handleOpenLogview,
  useDashboardBodySize,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed, getCurrentInstance, onMounted, ref } from 'vue';
import minimist from 'minimist';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfSetExtensionModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetExtensionModal.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';

import { columns, categoryRegisterConfig } from './config';
import path from 'path';
import {
  dealKfConfigValueByType,
  getIfProcessRunning,
  getIfProcessStopping,
  getTaskKfLocationByProcessId,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  graceDeleteProcess,
  graceStopProcess,
  Pm2ProcessStatusDetail,
  startTask,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  removeKfLocation,
  removeLog,
} from '@kungfu-trader/kungfu-js-api/actions';
import {
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const { extConfigs } = useExtConfigsRelated();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { processStatusData, processStatusDetailData } =
  useProcessStatusDetailData();

const setExtensionModalVisible = ref<boolean>(false);
const setTaskModalVisible = ref<boolean>(false);
const currentSelectedExtKey = ref<string>('');
const setTaskConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('TradingTask'),
  config: {} as KungfuApi.KfExtConfig,
});

const taskTypeKeys = computed(() => {
  return Object.keys(extConfigs.value['strategy'] || {});
});

const taskList = computed(() => {
  const taskCGs = taskTypeKeys.value.map((item) => {
    return `strategy_${item}`;
  });

  return Object.keys(processStatusDetailData.value)
    .filter((processId) => {
      return (
        taskCGs.findIndex((cg) => {
          return processId.indexOf(cg) === 0;
        }) !== -1
      );
    })
    .map((processId) => processStatusDetailData.value[processId])
    .sort((a, b) => (b?.name || '').localeCompare(a?.name || ''));
});
const { searchKeyword, tableData } =
  useTableSearchKeyword<Pm2ProcessStatusDetail>(taskList, ['name', 'args']);

const { dealRowClassName, setCurrentGlobalKfLocation } =
  useCurrentGlobalKfLocation(window.watcher);

const app = getCurrentInstance();

function handleOpenSetTaskDialog() {
  setExtensionModalVisible.value = true;
}

function handleOpenSetTaskModal(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedExtKey: string,
  taskConfig?: Record<string, KungfuApi.KfConfigValue>,
) {
  if (selectedExtKey === '') {
    error(t('tradingTaskConfig.key_inexistence'));
    return;
  }

  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    selectedExtKey
  ];

  if (!extConfig) {
    error(`${selectedExtKey} ${t('tradingTaskConfig.plugin_inexistence')}`);
    return;
  }

  currentSelectedExtKey.value = selectedExtKey;
  setTaskConfigPayload.value.type = type;
  setTaskConfigPayload.value.title = `${extConfig.name}`;
  setTaskConfigPayload.value.config = extConfig;
  setTaskConfigPayload.value.initValue = undefined;

  if (type === 'update') {
    if (taskConfig) {
      setTaskConfigPayload.value.initValue = taskConfig;
    }
  }

  if (!extConfig?.settings?.length) {
    error(
      `${t('tradingTaskConfig.configuration_inexistence')} ${
        extConfig?.name || selectedExtKey
      } package.json`,
    );
    return;
  }

  setTaskModalVisible.value = true;
}

function handleSwitchProcessStatusResolved(
  checked: boolean,
  event: MouseEvent,
  record: Pm2ProcessStatusDetail,
) {
  event.stopPropagation();

  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }

  if (!checked) {
    return graceStopProcess(
      window.watcher,
      taskLocation,
      processStatusData.value,
    )
      .then(() => {
        success();
      })
      .catch((err: Error) => {
        error(err.message || t('operation_failed'));
      });
  }

  const extKey = taskLocation.group;
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    error(`${extKey} ${t('tradingTaskConfig.plugin_inexistence')}`);
    return;
  }

  if (!extConfig.extPath) {
    error(
      `${t('tradingTaskConfig.configuration_inexistence')} ${
        extConfig?.name
      } .so`,
    );
    return;
  }

  const soPath = path.join(extConfig.extPath, extKey);
  const args = minimist(record.args as string[])['a'] || '';
  return startTask(taskLocation, soPath, args)
    .catch((err: Error) => error(err.message))
    .then(() => {
      success();
    })
    .catch((err: Error) => {
      error(err.message || t('operation_failed'));
    });
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
  const taskLocation: KungfuApi.KfLocation = {
    category: 'strategy',
    group: extKey,
    name: new Date().getTime().toString(),
    mode: 'LIVE',
  };

  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    error(`${extKey} ${t('tradingTaskConfig.plugin_inexistence')}`);
    return;
  }

  if (!extConfig.extPath) {
    error(
      `${t('tradingTaskConfig.configuration_inexistence')} ${
        extConfig?.name
      } .so`,
    );
    return;
  }

  const args: string = toArgs(extConfig.settings, formState);
  const soPath = path.join(extConfig.extPath, extKey);
  return ensureRemoveTask(taskLocation)
    .then(() => startTask(taskLocation, soPath, args))
    .then(() => {
      success();
    })
    .catch((err: Error) => error(err.message || t('operation_failed')));
}

function handleOpenLogviewResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }
  handleOpenLogview(taskLocation);
}

function handleRemoveTask(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }

  confirmModal(
    `${t('tradingTaskConfig.delete_task')} ${record.name}`,
    `${t('tradingTaskConfig.delete_task')} ${record.name}, ${t(
      'tradingTaskConfig.delete_task_content',
    )}`,
  ).then(() => {
    return ensureRemoveTask(taskLocation);
  });
}

function ensureRemoveTask(taskLocation: KungfuApi.KfLocation) {
  return graceDeleteProcess(
    window.watcher,
    taskLocation,
    processStatusData.value,
  )
    .then(() => removeKfLocation(taskLocation))
    .then(() => removeLog(taskLocation))
    .catch((err) => {
      console.error(err);
    });
}

function toArgs(
  settings: KungfuApi.KfConfigItem[],
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  return settings
    .filter((item) => {
      return formState[item.key] !== undefined;
    })
    .map((item) => {
      return `${item.key}=${formState[item.key]}`;
    })
    .join(';');
}

function fromArgs(args: string[]): Record<string, KungfuApi.KfConfigValue> {
  const taskArgs = minimist(args)['a'] || '';
  const data = getDataByArgs(taskArgs);
  return data;
}

function dealArgs(record: Pm2ProcessStatusDetail): string {
  const taskKfLocation = getTaskKfLocationByProcessId(record?.name || '');
  const taskArgs = minimist(record.args as string[])['a'] || '';
  if (!taskKfLocation) {
    return taskArgs.split(';').join(' ');
  }

  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    taskKfLocation.group
  ];
  if (!extConfig || !extConfig.settings) {
    return taskArgs.split(';').join(' ');
  }

  const data = getDataByArgs(taskArgs);
  return extConfig.settings
    .filter((item) => item.primary && data[item.key] !== undefined)
    .map((item) => {
      return dealKfConfigValueByType(item.type, data[item.key]);
    })
    .join(' ');
}

function getDataByArgs(taskArgs: string): Record<string, string> {
  return taskArgs.split(';').reduce((data, pair) => {
    const [key, value] = pair.split('=');
    data[key] = value;
    return data;
  }, {} as Record<string, string>);
}

function customRowResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }
  const locationResolved: KungfuApi.KfExtraLocation =
    resolveRowRecord(taskLocation);

  return {
    onClick: () => {
      setCurrentGlobalKfLocation(locationResolved);
    },
  };
}

function dealRowClassNameResolved(record: Pm2ProcessStatusDetail): string {
  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return '';
  }
  const locationResolved: KungfuApi.KfExtraLocation =
    resolveRowRecord(taskLocation);

  return dealRowClassName(locationResolved);
}

function resolveRowRecord(
  taskLocation: KungfuApi.KfLocation,
): KungfuApi.KfExtraLocation {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: categoryRegisterConfig.name,
    group: taskLocation?.group || '',
    name: taskLocation?.name || '',
    mode: 'LIVE',
  };
  return locationResolved;
}

onMounted(() => {
  if (app?.proxy && app.proxy.$globalCategoryRegister) {
    app.proxy.$globalCategoryRegister.register(categoryRegisterConfig);
  }
});
</script>

<template>
  <div class="kf-task-strategy__wrap kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetTaskDialog"
          >
            {{ $t('tradingTaskConfig.add_task') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableData"
        size="small"
        :pagination="false"
        :rowClassName="dealRowClassNameResolved"
        :customRow="customRowResolved"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :defaultExpandAllRows="true"
        :emptyText="$t('empty_text')"
      >
        <template #bodyCell="{ column, record }">
          <template v-if="column.dataIndex === 'processStatus'">
            <a-switch
              size="small"
              :checked="getIfProcessRunning(processStatusData, record.name)"
              :loading="getIfProcessStopping(processStatusData, record.name)"
              @click="(checked: boolean, Event: MouseEvent) => handleSwitchProcessStatusResolved(checked, Event, record)"
            ></a-switch>
          </template>
          <template v-if="column.dataIndex === 'args'">
            {{ dealArgs(record) }}
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogviewResolved(record)"
              />
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="
                  handleOpenSetTaskModal(
                    'update',
                    getTaskKfLocationByProcessId(record?.name || '')?.group ||
                      '',
                    fromArgs(record.args),
                  )
                "
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveTask(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
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
      :primaryKeyUnderline="true"
      @confirm="handleConfirmAddUpdateTask($event, currentSelectedExtKey)"
    ></KfSetByConfigModal>
  </div>
</template>
<style lang="less">
.kf-task-strategy__wrap {
  height: 100%;
}
</style>
