<script lang="ts" setup>
import {
  handleOpenLogview,
  useDashboardBodySize,
  useExtConfigsRelated,
  useProcessStatusDetailData,
  useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import { computed, ref } from 'vue';
import minimist from 'minimist';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfSetExtensionModal from '@renderer/components/public/KfSetExtensionModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';

import { columns } from './config';
import { message, Modal } from 'ant-design-vue';
import path from 'path';
import {
  dealKfConfigValueByType,
  getIfProcessRunning,
  getTaskKfLocationByProcessId,
  transformSearchInstrumentResultToInstrument,
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

const { extConfigs } = useExtConfigsRelated();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { processStatusData, processStatusDetailData } =
  useProcessStatusDetailData();

const setExtensionModalVisible = ref<boolean>(false);
const setTaskModalVisible = ref<boolean>(false);
const currentSelectedExtKey = ref<string>('');
const setTaskConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: '交易任务',
  config: {} as KungfuApi.KfExtConfig,
});

const taskTypeKeys = computed(() => {
  return Object.keys(extConfigs.data['strategy'] || {});
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
    .sort((a, b) => b.name.localeCompare(a.name));
});

const { searchKeyword, tableData } =
  useTableSearchKeyword<Pm2ProcessStatusDetail>(taskList, ['name', 'args']);

function handleOpenSetTaskDialog() {
  setExtensionModalVisible.value = true;
}

function handleOpenSetTaskModal(
  type = 'add' as KungfuApi.ModalChangeType,
  selectedExtKey: string,
  taskConfig?: Record<string, KungfuApi.KfConfigValue>,
) {
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    selectedExtKey
  ];

  if (!extConfig) {
    message.error(`${selectedExtKey} 交易任务插件不存在`);
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
    message.error(
      `配置项不存在, 请检查 ${extConfig?.name || selectedExtKey} package.json`,
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

  const taskLocation = getTaskKfLocationByProcessId(record.name);
  if (!taskLocation) {
    message.error(`${record.name} 不是合法交易任务进程ID`);
    return;
  }

  if (!checked) {
    return graceStopProcess(
      window.watcher,
      taskLocation,
      processStatusData.value,
    )
      .then(() => {
        message.success('操作成功');
      })
      .catch((err: Error) => {
        message.error(err.message || '操作失败');
      });
  }

  const extKey = taskLocation.group;
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    message.error(`${extKey} 交易任务插件不存在`);
    return;
  }

  if (!extConfig.extPath) {
    message.error(`配置项不存在, 请检查 ${extConfig?.name} .so`);
    return;
  }

  const soPath = path.join(extConfig.extPath, extKey);
  const args = minimist(record.args as string[])['a'] || '';
  return startTask(taskLocation, soPath, args)
    .catch((err: Error) => message.error(err.message))
    .then(() => {
      message.success('操作成功');
    })
    .catch((err: Error) => {
      message.error(err.message || '操作失败');
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

  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    message.error(`${extKey} 交易任务插件不存在`);
    return;
  }

  if (!extConfig.extPath) {
    message.error(`配置项不存在, 请检查 ${extConfig?.name} .so`);
    return;
  }

  const args: string = toArgs(extConfig.settings, formState);
  const soPath = path.join(extConfig.extPath, extKey);
  return ensureRemoveTask(taskLocation)
    .then(() => startTask(taskLocation, soPath, args))
    .then(() => {
      message.success('操作成功');
    })
    .catch((err: Error) => message.error(err.message || '操作失败'));
}

function handleOpenLogviewResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record.name);
  if (!taskLocation) {
    message.error(`${record.name} 不是合法交易任务进程ID`);
    return;
  }
  handleOpenLogview(taskLocation);
}

function handleRemoveTask(record: Pm2ProcessStatusDetail) {
  const taskLocation = getTaskKfLocationByProcessId(record.name);
  if (!taskLocation) {
    message.error(`${record.name} 不是合法交易任务进程ID`);
    return;
  }

  Modal.confirm({
    title: `删除交易任务 ${record.name}`,
    content: `删除交易任务 ${record.name}, 所有数据, 如果该交易任务正在运行, 也将停止进程, 确认删除`,
    okText: '确认',
    cancelText: '取消',
    onOk() {
      return ensureRemoveTask(taskLocation);
    },
  });
}

function ensureRemoveTask(taskLocation: KungfuApi.KfLocation) {
  return graceDeleteProcess(
    window.watcher,
    taskLocation,
    processStatusData.value,
  )
    .then(() => removeKfLocation(taskLocation))
    .then(() => removeLog(taskLocation));
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
      if (item.type === 'instrument') {
        const instrumentResolved = transformSearchInstrumentResultToInstrument(
          formState[item.key],
        );

        if (!instrumentResolved) {
          return `${item.key}=${formState[item.key]}`;
        }

        const { exchangeId, instrumentId } = instrumentResolved;
        return `${item.key}=${exchangeId}_${instrumentId}`;
      }
      return `${item.key}=${formState[item.key]}`;
    })
    .join(path.delimiter);
}

function fromArgs(args: string[]): Record<string, KungfuApi.KfConfigValue> {
  const taskArgs = minimist(args)['a'] || '';
  const data = getDataByArgs(taskArgs);
  return data;
}

function dealArgs(record: Pm2ProcessStatusDetail): string {
  const taskKfLocation = getTaskKfLocationByProcessId(record.name);
  const taskArgs = minimist(record.args as string[])['a'] || '';
  if (!taskKfLocation) {
    return taskArgs.split(path.delimiter).join(' ');
  }

  const extConfig: KungfuApi.KfExtConfig = (extConfigs.data['strategy'] || {})[
    taskKfLocation.group
  ];
  if (!extConfig || !extConfig.settings) {
    return taskArgs.split(path.delimiter).join(' ');
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
  return taskArgs.split(path.delimiter).reduce((data, pair) => {
    const [key, value] = pair.split('=');
    data[key] = value;
    return data;
  }, {} as Record<string, string>);
}
</script>

<template>
  <div class="kf-task-strategy__wrap kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            placeholder="关键字"
            style="width: 120px"
          />
        </KfDashboardItem>
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
        :data-source="tableData"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :defaultExpandAllRows="true"
        emptyText="暂无数据"
      >
        <template #bodyCell="{ column, record }">
          <template v-if="column.dataIndex === 'processStatus'">
            <a-switch
              size="small"
              :checked="getIfProcessRunning(processStatusData, record.name)"
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
                    getTaskKfLocationByProcessId(record.name)?.group,
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
