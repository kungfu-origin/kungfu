<script setup lang="ts">
import { getCurrentInstance, toRefs } from 'vue';
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
const app = getCurrentInstance();


const props = withDefaults(
  defineProps<{
    visible: boolean;
    formState: KungfuApi.MakeOrderInput
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
const { formState } = toRefs(props)
console.log(formState.value);

function handleConfirm() {
    app && app.emit('confirm');
    closeModal();
}
</script>

<template>
  <a-modal
    class="kf-order-confirm-modal"
    :width="420"
    v-model:visible="modalVisible"
    title="下单确认"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleConfirm"
  >
    <div class="confirm-content">
      <div class="confirm-item" v-for="(value, key) in formState">
        <span class="item-name">{{key}}: </span>
        <span class="item-value">{{value}}</span>
      </div>
    </div>
  </a-modal>
</template>
<style lang="less">
.kf-order-confirm-modal {
  .confirm-content {
    display: flex;
    flex-direction: column;
    flex-wrap: wrap;
    justify-content: left;
    .confirm-item {
      .item-name {
        font-size: 14px;
        color: rgba(255, 255, 255, 0.45);
        margin-right: 10px;
      }
      .item-value {
        font-size: 14px;
        color: rgba(255, 255, 255, 0.85);
      }
    }
  }
}
</style>