<script setup lang="ts">
import { computed, getCurrentInstance, reactive, ref } from 'vue';
import {
  initFormStateByConfig,
  useModalVisible,
} from '@renderer/assets/methods/uiUtils';

import { buildIdByKeysFromKfConfigSettings } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import KfConfigSettingsForm from '@renderer/components/public/KfConfigSettingsForm.vue';

const props = withDefaults(
  defineProps<{
    visible: boolean;
    payload: KungfuApi.SetKfConfigPayload;
    width?: number;
    primaryKeyAvoidRepeatCompareExtra?: string;
    primaryKeyAvoidRepeatCompareTarget?: string[];
  }>(),
  {
    visible: false,
    payload: () => ({} as KungfuApi.SetKfConfigPayload),
    width: 520,
    primaryKeyAvoidRepeatCompareTarget: () => [],
    primaryKeyAvoidRepeatCompareExtra: '',
  },
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
  initFormStateByConfig(
    props.payload.config?.settings || [],
    props.payload.initValue,
  ),
);

const titleResolved = computed(() => {
  return `${props.payload.type === 'add' ? '添加' : '设置'} ${
    props.payload.title
  }`;
});

function handleConfirm(): void {
  formRef.value
    .validate()
    .then(() => {
      const primaryKeys: string[] = (props.payload.config?.settings || [])
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
      :configSettings="payload.config?.settings || []"
      :changeType="payload.type"
      :primaryKeyAvoidRepeatCompareTarget="primaryKeyAvoidRepeatCompareTarget"
      :primaryKeyAvoidRepeatCompareExtra="primaryKeyAvoidRepeatCompareExtra"
    ></KfConfigSettingsForm>
  </a-modal>
</template>

<style lang="less"></style>
