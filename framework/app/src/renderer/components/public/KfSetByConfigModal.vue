<script setup lang="ts">
import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
  watch,
  nextTick,
} from 'vue';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';

import {
  buildIdByKeysFromKfConfigSettings,
  initFormStateByConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import KfConfigSettingsForm from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfConfigSettingsForm.vue';

const props = withDefaults(
  defineProps<{
    visible: boolean;
    payload: KungfuApi.SetKfConfigPayload;
    width?: number;
    primaryKeyUnderline?: boolean;
    primaryKeyAvoidRepeatCompareExtra?: string;
    primaryKeyAvoidRepeatCompareTarget?: string[];
  }>(),
  {
    visible: false,
    payload: () => ({} as KungfuApi.SetKfConfigPayload),
    width: 520,
    primaryKeyUnderline: false,
    primaryKeyAvoidRepeatCompareTarget: () => [],
    primaryKeyAvoidRepeatCompareExtra: '',
  },
);

const configSettings = ref<KungfuApi.KfConfigItem[]>(
  props.payload.config?.settings || [],
);

defineEmits<{
  (
    e: 'confirm',
    data: {
      formState: Record<string, KungfuApi.KfConfigValue>;
      idByPrimaryKeys: string;
      changeType: KungfuApi.ModalChangeType;
    },
  ): void;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const app = getCurrentInstance();
const { modalVisible, closeModal } = useModalVisible(props.visible);
const formRef = ref();
const formState = ref<Record<string, KungfuApi.KfConfigValue>>(
  initFormStateByConfig(configSettings.value || [], props.payload.initValue),
);

const titleResolved = computed(() => {
  return `${props.payload.type === 'add' ? '添加' : '设置'} ${
    props.payload.title
  }`;
});

watch(formState.value, (val) => {
  if (app?.proxy) {
    app?.proxy.$globalBus.next({
      tag: 'input:currentConfigModal',
      category: props.payload.config.category,
      extKey: props.payload.config.key,
      formState: val,
    });
  }
});

onMounted(() => {
  nextTick().then(() => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'ready:currentConfigModal',
        category: props.payload.config.category,
        extKey: props.payload.config.key,
        initValue: props.payload.initValue,
      });
    }
  });

  if (app?.proxy) {
    const subscription = app?.proxy.$globalBus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'update:currentConfigModalConfigSettings') {
        if (data.configSettings) {
          formState.value = initFormStateByConfig(data.configSettings || [], {
            ...formState.value,
            ...props.payload.initValue,
          });
          nextTick().then(() => {
            configSettings.value = data.configSettings;
          });
        }
      }
    });

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

function handleConfirm(): void {
  formRef.value
    .validate()
    .then(() => {
      const primaryKeys: string[] = (configSettings.value || [])
        .filter((item) => item.primary)
        .map((item) => item.key);

      const idByPrimaryKeys = buildIdByKeysFromKfConfigSettings(
        formState.value,
        primaryKeys,
      );
      app &&
        app.emit('confirm', {
          formState: formState.value,
          idByPrimaryKeys,
          changeType: props.payload.type,
        });
    })
    .then(() => {
      closeModal();
    })
    .catch((err: Error) => {
      console.error(err);
    });
}
</script>
<template>
  <a-modal
    :width="width"
    class="kf-set-by-config-modal"
    v-model:visible="modalVisible"
    :title="titleResolved"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <KfConfigSettingsForm
      ref="formRef"
      v-model:formState="formState"
      :configSettings="configSettings"
      :changeType="payload.type"
      :primaryKeyUnderline="primaryKeyUnderline"
      :primaryKeyAvoidRepeatCompareTarget="primaryKeyAvoidRepeatCompareTarget"
      :primaryKeyAvoidRepeatCompareExtra="primaryKeyAvoidRepeatCompareExtra"
    ></KfConfigSettingsForm>
  </a-modal>
</template>

<style lang="less"></style>
