<script setup lang="ts">
import { ref, onMounted, getCurrentInstance } from 'vue';

import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useLanguage } from '@kungfu-trader/kungfu-js-api/language';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;

const props = withDefaults(
  defineProps<{
    visible: boolean;
  }>(),
  {
    visible: false,
  },
);

defineEmits<{
  (e: 'confirm', sourceId: string): void;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const app = getCurrentInstance();
const selectedTransType = ref<string>('between_nodes');
const availTransRecordList = ref([
  {
    key: 'between_nodes',
    name: 'HTS -> HTS',
  },
  {
    key: 'tranc_in',
    name: `${t('fund_trans.centralized_counter')} -> HTS`,
  },
]);

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { isLanguageKeyAvailable } = useLanguage();

onMounted(() => {
  if (selectedTransType.value === 'between_nodes') {
    if (availTransRecordList.value.length) {
      selectedTransType.value = availTransRecordList.value[0].key;
    }
  }
});

function handleConfirm() {
  app && app.emit('confirm', selectedTransType.value);
  closeModal();
}
</script>
<template>
  <a-modal
    v-model:visible="modalVisible"
    class="kf-set-source-modal"
    :width="500"
    :title="$t('fund_trans.modal_title')"
    :destroy-on-close="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <p
      :style="{
        'font-size': '16px',
      }"
    >
      {{ $t('fund_trans.trans_selection') }}
    </p>
    <a-radio-group v-model:value="selectedTransType">
      <a-radio
        v-for="item in availTransRecordList"
        :key="item.key"
        :value="item.key"
        :style="{
          'min-height': '36px',
          'line-height': '36px',
          'font-size': '16px',
          'min-width': '45%',
        }"
      >
        <span class="source-name__txt">
          {{ isLanguageKeyAvailable(item.name) ? $t(item.name) : item.name }}
        </span>
      </a-radio>
    </a-radio-group>
  </a-modal>
</template>
<style lang="less">
.kf-set-source-modal {
  .source-name__txt {
    margin-right: 8px;
  }

  .source-id__txt {
    margin-right: 8px;
    font-size: 14px;
    color: @text-color-secondary;
  }
}
</style>
