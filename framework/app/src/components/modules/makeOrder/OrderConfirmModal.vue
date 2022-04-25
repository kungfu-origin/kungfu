<script setup lang="ts">
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { InstrumentTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { ref, toRefs, computed, getCurrentInstance } from 'vue';
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
  return volume.value
    ? Math.floor(+curOrderVolume.value / (+volume.value || 100))
    : '--';
});

defineEmits<{
  (e: 'confirm', volumeList: number[]): Promise<void>;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { curOrderType } = props;
const { curOrderVolume } = toRefs(props);

const volume = ref<number>(100);
volume.value = curOrderType === InstrumentTypeEnum.stock ? 100 : 1;

function handleConfirm() {
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
    title="拆单"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <a-row class="apart-input">
      <a-col>
        <a-statistic :value="curOrderVolume" title="总下单量" />
        <a-input-group compact style="margin-top: 10px" class="input-content">
          <span>{{ '每次下单量: ' }}</span>
          <a-input-number
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
          :value="curOrderVolume % volume === 0 ? orderNumber : orderNumber + 1"
          :valueStyle="{ fontSize: '35px' }"
          title="下单次数"
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
  .input-content {
    span {
      line-height: 32px;
      margin-right: 10px !important;
    }
  }
}
</style>
