import { getCurrentInstance, onBeforeUnmount, onMounted, ref, Ref } from 'vue';
import path from 'path';
import { message } from 'ant-design-vue';
import {
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { kfConfigItemsToProcessArgs } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import globalBus from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/globalBus';
import { startTradingTask } from '@kungfu-trader/kungfu-js-api/actions/tradingTask';

export const useTradingTask = (): {
  setTradingTaskModalVisible: Ref<boolean>;
  currentSelectedTradingTaskExtKey: Ref<string>;
  setTradingTaskConfigPayload: Ref<KungfuApi.SetKfConfigPayload>;
  triggerAddTradingTask: (
    extKey: string,
    payload: KungfuApi.SetKfConfigPayload,
  ) => void;
  handleOpenSetTradingTaskModal: (
    type: KungfuApi.ModalChangeType,
    selectedExtKey: string,
    taskInitValue?: Record<string, KungfuApi.KfConfigValue>,
  ) => void;
  handleConfirmAddUpdateTask: (
    data: {
      formState: Record<string, KungfuApi.KfConfigValue>;
      idByPrimaryKeys: string;
      configSettings: KungfuApi.KfConfigItem[];
      changeType: KungfuApi.ModalChangeType;
    },
    extKey: string,
  ) => void;
} => {
  const setTradingTaskModalVisible = ref<boolean>(false);
  const currentSelectedTradingTaskExtKey = ref<string>('');
  const setTradingTaskConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
    type: 'add',
    title: '交易任务',
    config: {} as KungfuApi.KfExtConfig,
  });
  const { extConfigs } = useExtConfigsRelated();
  const { processStatusData } = useProcessStatusDetailData();
  const app = getCurrentInstance();
  const tradingTaskCategory = 'strategy';

  const handleOpenSetTradingTaskModal = (
    type = 'add' as KungfuApi.ModalChangeType,
    selectedExtKey: string,
    taskInitValue?: Record<string, KungfuApi.KfConfigValue>,
  ) => {
    if (selectedExtKey === '') {
      message.error(`交易任务插件 key 不存在`);
      return;
    }

    const extConfig: KungfuApi.KfExtConfig = (extConfigs.value[
      tradingTaskCategory
    ] || {})[selectedExtKey];

    if (!extConfig) {
      message.error(`${selectedExtKey} 交易任务插件不存在`);
      return;
    }

    currentSelectedTradingTaskExtKey.value = selectedExtKey;
    setTradingTaskConfigPayload.value.type = type;
    setTradingTaskConfigPayload.value.title = `${extConfig.name}`;
    setTradingTaskConfigPayload.value.config = extConfig;
    setTradingTaskConfigPayload.value.initValue = undefined;

    if (taskInitValue) {
      setTradingTaskConfigPayload.value.initValue = taskInitValue;
    }

    if (!extConfig?.settings?.length) {
      message.error(
        `配置项不存在, 请检查 ${
          extConfig?.name || selectedExtKey
        } package.json`,
      );
      return;
    }

    triggerAddTradingTask(selectedExtKey, setTradingTaskConfigPayload.value);
  };

  const triggerAddTradingTask = (
    extKey: string,
    payload: KungfuApi.SetKfConfigPayload,
  ) => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'setTradingTask',
        extKey,
        payload,
      });
    }
  };

  const handleConfirmAddUpdateTask = (
    data: {
      formState: Record<string, KungfuApi.KfConfigValue>;
      configSettings: KungfuApi.KfConfigItem[];
      idByPrimaryKeys: string;
      changeType: KungfuApi.ModalChangeType;
    },
    extKey: string,
  ) => {
    const { formState } = data;
    const taskLocation: KungfuApi.KfLocation = {
      category: 'strategy',
      group: extKey,
      name: new Date().getTime().toString(),
      mode: 'LIVE',
    };

    const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] ||
      {})[extKey];

    if (!extConfig) {
      message.error(`${extKey} 交易任务插件不存在`);
      return;
    }

    if (!extConfig.extPath) {
      message.error(`配置项不存在, 请检查 ${extConfig?.name} .so`);
      return;
    }

    const args: string = kfConfigItemsToProcessArgs(
      data.configSettings || extConfig.settings || [],
      formState,
    );
    const soPath = path.join(extConfig.extPath, extKey);
    return startTradingTask(
      window.watcher,
      taskLocation,
      processStatusData.value,
      soPath,
      args,
    )
      .then(() => {
        message.success('操作成功');
      })
      .catch((err: Error) => message.error(err.message || '操作失败'));
  };

  onMounted(() => {
    if (app?.proxy) {
      const subscription = globalBus.subscribe((data: KfBusEvent) => {
        if (data.tag === 'setTradingTask') {
          currentSelectedTradingTaskExtKey.value = data.extKey;
          setTradingTaskConfigPayload.value = data.payload;
          setTradingTaskModalVisible.value = true;
        }
      });

      onBeforeUnmount(() => {
        subscription.unsubscribe();
      });
    }
  });

  return {
    setTradingTaskModalVisible,
    currentSelectedTradingTaskExtKey,
    setTradingTaskConfigPayload,
    triggerAddTradingTask,
    handleOpenSetTradingTaskModal,
    handleConfirmAddUpdateTask,
  };
};
