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
const selectedExtension = ref<string>('between_nodes');
const availExtensionList = ref([
  {
    key: 'between_nodes',
    name: 'HTS -> HTS',
  },
  {
    key: 'tranc_in',
    name: `${t('account_move.centralized_counter')} -> HTS`,
  },
]);

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { isLanguageKeyAvailable } = useLanguage();

onMounted(() => {
  if (selectedExtension.value === 'between_nodes') {
    if (availExtensionList.value.length) {
      selectedExtension.value = availExtensionList.value[0].key;
    }
  }
});

function handleConfirm() {
  app && app.emit('confirm', selectedExtension.value);
  closeModal();
}
</script>
<template>
  <a-modal
    v-model:visible="modalVisible"
    class="kf-set-source-modal"
    :width="500"
    :title="$t('account_move.modal_title')"
    :destroy-on-close="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <p>{{ $t('account_move.trans_selection') }}</p>
    <a-radio-group v-model:value="selectedExtension">
      <a-radio
        v-for="item in availExtensionList"
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
    // font-weight: bold;
  }

  .source-id__txt {
    margin-right: 8px;
    font-size: 14px;
    color: @text-color-secondary;
  }
}
</style>
