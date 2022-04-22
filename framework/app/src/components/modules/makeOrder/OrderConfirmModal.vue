<script setup lang="ts">
// import { getCurrentInstance } from 'vue';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { ref, toRefs, computed, getCurrentInstance } from 'vue';
const app = getCurrentInstance();

const props = withDefaults(
  defineProps<{
    visible: boolean;
    curOrderVolume: number;
  }>(),
  {
    visible: false,
  },
);
const orderNumber = computed(() => {
  if (apartType.value === 'orderSize') {
    return Math.floor(+curOrderVolume.value / (+volume.value || 100));
  }
  return +curOrderVolume.value % (+volume.value || 100) == 0
    ? Math.floor(+curOrderVolume.value / (+volume.value || 100))
    : Math.floor(+curOrderVolume.value / (+volume.value || 100)) + 1;
});

defineEmits<{
  (
    e: 'confirm',
    { volume, count }: { volume: number; count: number },
  ): Promise<void>;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { curOrderVolume } = toRefs(props);

const volume = ref<number>(100);
const apartType = ref<string>('orderSize');
function handleConfirm() {
  app &&
    app.emit('confirm', {
      volume: volume.value,
      count: orderNumber.value,
    });
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
        <a-input-group compact style="margin-top: 10px">
          <a-select v-model:value="apartType">
            <a-select-option value="orderSize">每次下单量</a-select-option>
            <a-select-option value="orderCount">下单次数</a-select-option>
          </a-select>
          <a-input-number
            :precision="0"
            step="100"
            v-model:value="volume"
            :max="
              apartType === 'orderSize'
                ? curOrderVolume
                : Math.ceil(+curOrderVolume / +orderNumber)
            "
            :min="1"
          ></a-input-number>
        </a-input-group>
      </a-col>
      <a-col class="apart-result">
        <a-statistic
          class="apart-result-statistic"
          :value="curOrderVolume % volume === 0 ? orderNumber : orderNumber + 1"
          :valueStyle="{ fontSize: '35px' }"
          :title="apartType === 'orderSize' ? '下单次数' : '每次下单量'"
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
}
</style>
