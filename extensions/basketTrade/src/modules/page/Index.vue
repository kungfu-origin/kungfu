<template>
  <div class="kf-basket-trade__warp">
    <KfRowColIter :board-id="0" :closable="false"></KfRowColIter>
  </div>
  <KfSetByConfigModal
    v-if="showMakeOrderModal"
    v-model:visible="showMakeOrderModal"
    :payload="makeBasketOrderConfigPayload"
    @close="handleCloseModal"
    @confirm="({ formState }) => handleConfirmMakeBasketOrder(formState)"
  ></KfSetByConfigModal>
</template>

<script lang="ts" setup>
import { getCurrentInstance, onBeforeUnmount, onMounted } from 'vue';

import KfRowColIter from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfRowColIter.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getBasketTradeBoardsMap,
  registerComponents,
  setBasketTradeBoardsMap,
} from '../../utils';
import { defaultBasketTradeBoardsMap } from '../../config';
import { componentsMap } from '../../config/componentsConfig';
import { useMakeBasketOrderFormModal } from '../../utils/basketTradeUtils';

const app = getCurrentInstance();
const { success } = messagePrompt();
const boardsMap: KfLayout.BoardsMap =
  getBasketTradeBoardsMap() || defaultBasketTradeBoardsMap;
const {
  showMakeOrderModal,
  handleCloseModal,
  makeBasketOrderConfigPayload,
  handleConfirmMakeBasketOrder,
} = useMakeBasketOrderFormModal();

if (app) {
  registerComponents(app.appContext.app, componentsMap);
}

onMounted(() => {
  useGlobalStore().initBoardsMap(boardsMap);

  const subscription = app?.proxy?.$globalBus.subscribe(
    (data: KfEvent.KfBusEvent) => {
      if (data.tag === 'main') {
        if (data.name === 'reset-main-dashboard') {
          useGlobalStore().initBoardsMap(defaultBasketTradeBoardsMap);
          success();
        }
      }

      if (data.name == 'record-before-quit') {
        setBasketTradeBoardsMap(useGlobalStore().boardsMap);
        window.watcher && window.watcher.quit();
      }
    },
  );

  onBeforeUnmount(() => {
    subscription?.unsubscribe();
    setBasketTradeBoardsMap(useGlobalStore().boardsMap);
  });
});
</script>

<style lang="less">
.kf-basket-trade__warp {
  height: 100%;
  width: 100%;

  .ant-spin.ant-spin-spinning {
    position: unset;
  }

  & > .kf-drag-row__warp {
    height: 100%;
  }
}
</style>
