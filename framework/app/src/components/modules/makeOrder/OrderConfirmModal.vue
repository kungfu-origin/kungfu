<script setup lang="ts">
// import { getCurrentInstance } from 'vue';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { ref, toRefs, computed, getCurrentInstance } from 'vue';
const app = getCurrentInstance();

const props = withDefaults(
  defineProps<{
    visible: boolean;
    orderMessage: number;
  }>(),
  {
    visible: false,
  },
);
const orderNumber = computed(() => {
  if (apartType.value === 'orderSize') {
    return Math.floor(+orderMessage.value / (+amount.value || 100))
  }
  return (+orderMessage.value % (+amount.value || 100) == 0) ? Math.floor(+orderMessage.value / (+amount.value || 100)) : Math.floor(+orderMessage.value / (+amount.value || 100)) + 1
})

defineEmits<{
  (e: 'confirm',  amount: number ): void;
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const { orderMessage } = toRefs(props);

const amount = ref<number>(100);
const apartType = ref<string>('orderSize')
function handleConfirm() {
  app && app.emit('confirm', apartType.value === 'orderSize' ? orderNumber.value : amount.value);
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
        <a-statistic
          :value="orderMessage"
          title="总下单量"
        />
        <a-input-group compact style="margin-top: 10px">
          <a-select v-model:value="apartType">
            <a-select-option value="orderSize">每次下单量</a-select-option>
            <a-select-option value="orderCount">下单次数</a-select-option>
          </a-select>
        <a-input-number
          :precision="0"
          step="100"
          v-model:value="amount"
          :max="apartType === 'orderSize' ? orderMessage : Math.ceil(+orderMessage / +orderNumber)"
          :min="1"
        ></a-input-number>
      </a-input-group>
      </a-col>
      <a-col class="apart-result">
        <a-statistic
          class="apart-result-statistic"
          :value="orderNumber"
          :valueStyle="{fontSize: '35px'}"
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
