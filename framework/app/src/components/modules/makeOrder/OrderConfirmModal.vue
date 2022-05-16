<script setup lang="ts">
import {
  messagePrompt,
  useModalVisible,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { ref, toRefs, computed, getCurrentInstance } from 'vue';
import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

const { error } = messagePrompt();
const app = getCurrentInstance();

const props = withDefaults(
  defineProps<{
    visible: boolean;
    curOrderVolume: number;
    curOrderType: InstrumentTypeEnum;
  }>(),
  {
    visible: false,
  },
);
const orderNumber = computed(() => {
  return volume.value ? Math.floor(+curOrderVolume.value / +volume.value) : 0;
});

defineEmits<{
  (e: 'confirm', volumeList: number[]): Promise<void>;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { curOrderType } = props;
const { curOrderVolume } = toRefs(props);

const shotable = curOrderType
  ? ShotableInstrumentTypes.includes(curOrderType)
  : false;
const defaultVolume = computed(() => {
  if (shotable) {
    return 1;
  }
  return 100;
});

const volume = ref<number>(defaultVolume.value);

function handleConfirm() {
  if (volume.value === null) {
    error(t('tradingConfig.no_empty'));
    return;
  }
  const remainder: number = curOrderVolume.value % +volume.value; // 剩余数量
  const volumeList: number[] = new Array(+orderNumber.value).fill(
    +volume.value,
  );

  if (remainder !== 0) {
    volumeList.push(remainder);
  }

  app && app.emit('confirm', volumeList);
  closeModal();
}
</script>

<template>
  <a-modal
    class="kf-order-confirm-modal"
    :width="420"
    v-model:visible="modalVisible"
    :title="$t('tradingConfig.apart_order')"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <a-row class="apart-input">
      <a-col>
        <a-statistic
          :value="curOrderVolume"
          :title="$t('tradingConfig.total_order_amount')"
        />
        <a-input-group compact style="margin-top: 10px" class="input-content">
          <span>{{ $t('tradingConfig.every_volume') }}:</span>
          <a-input-number
            class="input-number"
            :precision="0"
            step="1"
            v-model:value="volume"
            :max="curOrderVolume"
            :min="1"
          ></a-input-number>
        </a-input-group>
      </a-col>
      <a-col class="apart-result">
        <a-statistic
          class="apart-result-statistic"
          :value="
            volume
              ? curOrderVolume % volume === 0
                ? orderNumber
                : orderNumber + 1
              : '--'
          "
          :valueStyle="{ fontSize: '35px' }"
          :title="$t('tradingConfig.make_order_number')"
        />
      </a-col>
    </a-row>
  </a-modal>
</template>
<style lang="less">
.kf-order-confirm-modal {
  .apart-result {
    text-align: center;
    margin: auto;
  }
  .input-number {
    margin-left: 10px;
  }
  .input-content {
    span {
      line-height: 32px;
    }
  }
}
</style>
