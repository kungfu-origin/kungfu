<script setup lang="ts">
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import type { Dayjs } from 'dayjs';
import dayjs from 'dayjs';
import { getCurrentInstance, reactive, ref } from 'vue';

const app = getCurrentInstance();

const props = withDefaults(
  defineProps<{
    visible: boolean;
  }>(),
  {
    visible: false,
  },
);

defineEmits<{
  (e: 'confirm', formState: { date: string; dateType: HistoryDateEnum }): void;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const formState = reactive({
  date: ref<Dayjs | string>(dayjs().format()),
  dateType: HistoryDateEnum.naturalDate,
});

const formRef = ref();
function handleConfirm() {
  return formRef.value
    .validate()
    .then(() => {
      app && app.emit('confirm', formState);
      closeModal();
    })
    .catch((err: Error) => {
      console.error(err);
    });
}
</script>
<template>
  <a-modal
    class="kf-export-date-modal"
    :width="320"
    v-model:visible="modalVisible"
    title="选择日期"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <a-form ref="formRef" :model="formState" :colon="false">
      <a-form-item
        label="选择日期"
        name="date"
        required
        :rules="[
          {
            required: true,
            message: '该项为必填项',
            trigger: 'blur',
          },
        ]"
      >
        <a-date-picker
          v-model:value="formState.date"
          valueFormat="YYYYMMDD HH:mm:ss"
        ></a-date-picker>
      </a-form-item>
      <a-form-item
        label="日期类型"
        name="dateType"
        required
        :rules="[
          {
            type: 'number',
            required: true,
            message: '该项为必填项',
            trigger: 'blur',
          },
        ]"
      >
        <a-radio-group v-model:value="formState.dateType">
          <a-radio-button :value="HistoryDateEnum.naturalDate">
            自然日
          </a-radio-button>
          <a-radio-button :value="HistoryDateEnum.tradingDate">
            交易日
          </a-radio-button>
        </a-radio-group>
      </a-form-item>
    </a-form>
  </a-modal>
</template>
