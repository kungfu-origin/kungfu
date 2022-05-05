<template>
  <div class="kf-index__warp">
    <KfRowColIter :board-id="0" :closable="true"></KfRowColIter>
    <KfAddBoardModalVue
      v-if="addBoardModalVisible"
      v-model:visible="addBoardModalVisible"
      :targetBoardId="addBoardTargetBoardId"
    ></KfAddBoardModalVue>
  </div>
</template>

<script lang="ts">
import { defineComponent, onBeforeUnmount, ref } from 'vue';

import KfRowColIter from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfRowColIter.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import {
  defaultBoardsMap,
  getIndexBoardsMap,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/configs';
import { usePreStartAndQuitApp } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import KfAddBoardModalVue from '../../../components/public/KfAddBoardModal.vue';
import globalBus from '../../../assets/methods/globalBus';
import { messagePrompt } from '../../../assets/methods/uiUtils';
const { success } = messagePrompt();

export default defineComponent({
  name: 'Index',

  setup() {
    const boardsMap: KfLayout.BoardsMap =
      getIndexBoardsMap() || defaultBoardsMap;

    const { initBoardsMap } = useGlobalStore();
    initBoardsMap(boardsMap);

    const addBoardModalVisible = ref<boolean>(false);
    const addBoardTargetBoardId = ref<number>(-1);

    const subscription = globalBus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'addBoard') {
        addBoardModalVisible.value = true;
        addBoardTargetBoardId.value = data.boardId;
      }

      if (data.tag === 'main') {
        if (data.name === 'reset-main-dashboard') {
          initBoardsMap(defaultBoardsMap);
          success();
        }
      }
    });

    const { saveBoardsMap } = usePreStartAndQuitApp();
    onBeforeUnmount(() => {
      subscription.unsubscribe();
      saveBoardsMap();
    });

    return {
      addBoardModalVisible,
      addBoardTargetBoardId,
    };
  },

  components: {
    KfRowColIter,
    KfAddBoardModalVue,
  },
});
</script>

<style lang="less">
.kf-index__warp {
  height: 100%;
  width: 100%;

  & > .kf-drag-row__warp {
    height: 100%;
  }
}
</style>
