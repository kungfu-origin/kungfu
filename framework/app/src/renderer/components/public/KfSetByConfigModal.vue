<script setup lang="ts">
import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
  nextTick,
  toRaw,
} from 'vue';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';

import {
  buildIdByPrimaryKeysFromKfConfigSettings,
  initFormStateByConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import KfConfigSettingsForm from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfConfigSettingsForm.vue';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const props = withDefaults(
  defineProps<{
    visible: boolean;
    payload: KungfuApi.SetKfConfigPayload;
    width?: number;
    labelCol?: number;
    wrapperCol?: number;
    isPrimaryDisabled?: boolean;
    passPrimaryKeySpecialWordsVerify?: boolean;
    primaryKeyAvoidRepeatCompareExtra?: string;
    primaryKeyAvoidRepeatCompareTarget?: string[];
  }>(),
  {
    visible: false,
    payload: () => ({} as KungfuApi.SetKfConfigPayload),
    width: 520,
    labelCol: 6,
    wrapperCol: 14,
    isPrimaryDisabled: false,
    passPrimaryKeySpecialWordsVerify: false,
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
      configSettings: KungfuApi.KfConfigItem[];
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
  if (props.payload.type === 'add') {
    return `${t('add')} ${props.payload.title}`;
  } else if (props.payload.type === 'update') {
    return `${t('set')} ${props.payload.title}`;
  }

  return `${props.payload.title}`;
});

onMounted(() => {
  nextTick().then(() => {
    if (app?.proxy) {
      app?.proxy.$globalBus.next({
        tag: 'ready:currentConfigModal',
        category: props.payload.config.category,
        extKey: props.payload.config.key,
        initValue: toRaw(props.payload.initValue),
      });
    }
  });

  if (app?.proxy) {
    const subscription = app?.proxy.$globalBus.subscribe(
      (data: KfEvent.KfBusEvent) => {
        if (data.tag === 'update:currentConfigModalConfigSettings') {
          if (data.configSettings) {
            nextTick().then(() => {
              formState.value = initFormStateByConfig(
                data.configSettings || [],
                {
                  ...toRaw(props.payload.initValue),
                  ...toRaw(formState.value),
                },
              );

              configSettings.value = data.configSettings;
            });
          }
        }
      },
    );

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

      const idByPrimaryKeys = buildIdByPrimaryKeysFromKfConfigSettings(
        formState.value,
        primaryKeys,
      );
      app &&
        app.emit('confirm', {
          formState: formState.value,
          configSettings: configSettings.value,
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

function handleCancel(): void {
  app && app.emit('close');
  closeModal();
}

function handleFormStateChange(formState) {
  if (app?.proxy) {
    app?.proxy.$globalBus.next({
      tag: 'input:currentConfigModal',
      category: props.payload.config.category,
      extKey: props.payload.config.key,
      formState: toRaw(formState),
    });
  }
}
</script>
<template>
  <a-modal
    v-model:visible="modalVisible"
    :width="width"
    class="kf-set-by-config-modal"
    :title="titleResolved"
    :destroy-on-close="true"
    @cancel="handleCancel"
    @ok="handleConfirm"
  >
    <KfConfigSettingsForm
      ref="formRef"
      v-model:formState="formState"
      :config-settings="configSettings"
      :change-type="payload.type"
      :label-col="labelCol"
      :wrapper-col="wrapperCol"
      :is-primary-disabled="isPrimaryDisabled"
      :pass-primary-key-special-words-verify="passPrimaryKeySpecialWordsVerify"
      :primary-key-avoid-repeat-compare-target="
        primaryKeyAvoidRepeatCompareTarget
      "
      :primary-key-avoid-repeat-compare-extra="
        primaryKeyAvoidRepeatCompareExtra
      "
      @update:form-state="handleFormStateChange"
    ></KfConfigSettingsForm>
  </a-modal>
</template>

<style lang="less"></style>
