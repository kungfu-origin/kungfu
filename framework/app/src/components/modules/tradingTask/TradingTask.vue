<script lang="ts" setup>
import {
  confirmModal,
  handleOpenLogview,
  useDashboardBodySize,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed, ref } from 'vue';
import minimist from 'minimist';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfSetExtensionModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetExtensionModal.vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';

import { columns } from './config';
import path from 'path';
import {
  getIfProcessRunning,
  getIfProcessStopping,
  getTaskKfLocationByProcessId,
  fromProcessArgsToKfConfigItems,
  kfConfigItemsToArgsByPrimaryForShow,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  graceStopProcess,
  Pm2ProcessStatusDetail,
  startTask,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { ensureRemoveTradingTask } from '@kungfu-trader/kungfu-js-api/actions/tradingTask';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
import { useTradingTask } from './utils';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const { extConfigs } = useExtConfigsRelated();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { processStatusData, processStatusDetailData } =
  useProcessStatusDetailData();

const { handleOpenSetTradingTaskModal } = useTradingTask();

const setExtensionModalVisible = ref<boolean>(false);

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


function handleOpenSetTaskDialog() {
  setExtensionModalVisible.value = true;
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
      `${t('tradingTaskConfig.configuration_inexistence')} ${extConfig?.name}`,
    );
    return;
  }

  const soPath = path.join(extConfig.extPath, extKey);
  const args = minimist(record.args as string[])['a'] || '';
  const configSettings = parseTaskSettingsFromEnv(
    record.config_settings || '[]',
  );

  return startTask(taskLocation, soPath, args, configSettings)
    .catch((err: Error) => error(err.message))
    .then(() => {
      success();
    })
    .catch((err: Error) => {
      error(err.message || t('operation_failed'));
    });
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
    return ensureRemoveTradingTask(
      window.watcher,
      taskLocation,
      processStatusData.value,
    );
  });
}

function customRowResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }
  const locationResolved: KungfuApi.KfExtraLocation =
    resolveKfLocation(taskLocation);

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
    resolveKfLocation(taskLocation);

  return dealRowClassName(locationResolved);
}

function resolveKfLocation(
  taskLocation: KungfuApi.KfLocation,
): KungfuApi.KfExtraLocation {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: 'strategy',
    group: taskLocation?.group || '',
    name: taskLocation?.name || '',
    mode: 'live',
  };
  return locationResolved;
}

const taskArgsData: Record<string, string> = {};

function resolveArgs(record: Pm2ProcessStatusDetail) {
  const name = record?.name || '';
  if (taskArgsData[name]) {
    return taskArgsData[name];
  }

  const configSettings = parseTaskSettingsFromEnv(record.config_settings);
  const formState = fromProcessArgsToKfConfigItems(record.args || []);
  const argsForShow = kfConfigItemsToArgsByPrimaryForShow(
    configSettings,
    formState,
  );
  taskArgsData[name] = argsForShow;
  return argsForShow;
}

function parseTaskSettingsFromEnv(configSettingsEnv = '[]') {
  let configSettings: KungfuApi.KfConfigItem[] = [];

  try {
    configSettings = JSON.parse(configSettingsEnv) as KungfuApi.KfConfigItem[];
  } catch (err) {
    console.error(err.message);
  }
  return configSettings;
}

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
            {{ resolveArgs(record) }}
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
                  handleOpenSetTradingTaskModal(
                    'update',
                    getTaskKfLocationByProcessId(record?.name || '')?.group ||
                      '',
                    fromProcessArgsToKfConfigItems(record.args),
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
      @confirm="handleOpenSetTradingTaskModal('add', $event)"
    ></KfSetExtensionModal>
  </div>
</template>
<style lang="less">
.kf-task-strategy__wrap {
  height: 100%;
}
</style>
