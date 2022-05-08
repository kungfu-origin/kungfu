<script setup lang="ts">
import { ref, onMounted, computed, getCurrentInstance } from 'vue';

import {
  getInstrumentTypeData,
  getStrategyExtTypeData,
  getExtConfigList,
  isTdMd,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  KfCategoryTypes,
  InstrumentTypes,
  StrategyExtTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useExtConfigsRelated } from '../../assets/methods/actionsUtils';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

const props = withDefaults(
  defineProps<{
    visible: boolean;
    extensionType: KfCategoryTypes;
  }>(),
  {
    visible: false,
    extensionType: 'td',
  },
);

defineEmits<{
  (e: 'confirm', sourceId: string): void;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const app = getCurrentInstance();
const { extConfigs } = useExtConfigsRelated();
const selectedExtension = ref<string>('');
const availExtensionList = computed(() => {
  return getExtConfigList(extConfigs.value, props.extensionType);
});

const { modalVisible, closeModal } = useModalVisible(props.visible);

const modalTitle = computed(() => {
  if (props.extensionType === 'td' || props.extensionType === 'md') {
    return t('mdConfig.select_counter_api');
  } else if (props.extensionType === 'strategy') {
    return t('mdConfig.select_trade_task');
  } else {
    return t('mdConfig.select_plugin_type');
  }
});

onMounted(() => {
  if (selectedExtension.value === '') {
    if (availExtensionList.value.length) {
      selectedExtension.value = availExtensionList.value[0].key;
    }
  }
});

function handleConfirm() {
  app && app.emit('confirm', selectedExtension.value);
  closeModal();
}

function getKungfuTradeValueCommonDataByExtType(
  category: KfCategoryTypes,
  extType: InstrumentTypes | StrategyExtTypes,
) {
  if (isTdMd(category)) {
    return getInstrumentTypeData(extType as InstrumentTypes);
  }

  return getStrategyExtTypeData(extType as StrategyExtTypes);
}
</script>
<template>
  <a-modal
    class="kf-set-source-modal"
    :width="480"
    v-model:visible="modalVisible"
    :title="modalTitle"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <a-radio-group v-model:value="selectedExtension">
      <a-radio
        :value="item.key"
        :key="item.key"
        v-for="item in availExtensionList"
        :style="{
          height: '36px',
          'line-height': '36px',
          'font-size': '16px',
          'min-width': '45%',
        }"
      >
        <span class="source-id__txt">{{ item.name }}</span>
        <a-tag
          v-for="(extType, index) in item.type"
          :key="index"
          :color="
            getKungfuTradeValueCommonDataByExtType(extensionType, extType).color
          "
        >
          {{
            getKungfuTradeValueCommonDataByExtType(extensionType, extType).name
          }}
        </a-tag>
      </a-radio>
    </a-radio-group>
  </a-modal>
</template>
<style lang="less">
.kf-set-source-modal {
  .source-id__txt {
    margin-right: 8px;
  }
}
</style>
