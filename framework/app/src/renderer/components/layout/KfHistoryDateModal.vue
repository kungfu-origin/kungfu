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
    :title="$t('select_date')"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <a-form ref="formRef" :model="formState" :colon="false">
      <a-form-item
        :label="$t('select_date')"
        name="date"
        required
        :rules="[
          {
            required: true,
            message: $t('validate.mandatory'),
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
        :label="$t('date_type')"
        name="dateType"
        required
        :rules="[
          {
            type: 'number',
            required: true,
            message: $t('validate.mandatory'),
            trigger: 'blur',
          },
        ]"
      >
        <a-radio-group v-model:value="formState.dateType">
          <a-radio-button :value="HistoryDateEnum.naturalDate">
            {{ $t('natural_day') }}
          </a-radio-button>
          <a-radio-button :value="HistoryDateEnum.tradingDate">
            {{ $t('trading_day') }}
          </a-radio-button>
        </a-radio-group>
      </a-form-item>
    </a-form>
  </a-modal>
</template>
