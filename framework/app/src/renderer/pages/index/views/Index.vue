<template>
  <div class="kf-index__warp">
    <KfRowColIter
      v-if="boardsMap?.[0]?.children?.length"
      :board-id="0"
      :closable="true"
    ></KfRowColIter>
    <a-empty v-else class="kf-index__empty" :image="simpleImage">
      <template #description>
        <span>
          {{ $t('board_empty') }}
        </span>
      </template>
      <a-button type="primary" @click="handleAddBoardFromEmpty">
        {{ $t('add_board_now') }}
      </a-button>
    </a-empty>
    <KfAddBoardModalVue
      v-if="addBoardModalVisible"
      v-model:visible="addBoardModalVisible"
      :target-board-id="addBoardTargetBoardId"
    ></KfAddBoardModalVue>
  </div>
</template>

<script lang="ts">
import { defineComponent, onBeforeUnmount, ref } from 'vue';
import { storeToRefs } from 'pinia';

import KfRowColIter from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfRowColIter.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import {
  defaultBoardsMap,
  getIndexBoardsMap,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/configs';
import { usePreStartAndQuitApp } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import KfAddBoardModalVue from '../../../components/public/KfAddBoardModal.vue';
import { Empty } from 'ant-design-vue';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import { messagePrompt } from '../../../assets/methods/uiUtils';
const { success } = messagePrompt();

export default defineComponent({
  name: 'Index',

  components: {
    KfRowColIter,
    KfAddBoardModalVue,
  },

  setup() {
    const simpleImage = Empty.PRESENTED_IMAGE_SIMPLE;
    const { initBoardsMap } = useGlobalStore();
    const { boardsMap } = storeToRefs(useGlobalStore());

    const dealDefaultBoardsHook =
      globalThis.HookKeeper.getHooks().dealBoardsMap;
    const curBoardsMap: KfLayout.BoardsMap =
      getIndexBoardsMap() || dealDefaultBoardsHook.trigger(defaultBoardsMap);

    initBoardsMap(curBoardsMap);

    const addBoardModalVisible = ref<boolean>(false);
    const addBoardTargetBoardId = ref<number>(-1);
    const { saveBoardsMap } = usePreStartAndQuitApp();

    const subscription = globalBus.subscribe((data: KfEvent.KfBusEvent) => {
      if (data.tag === 'addBoard') {
        addBoardModalVisible.value = true;
        addBoardTargetBoardId.value = data.boardId;
      }

      if (data.tag === 'main') {
        if (data.name === 'reset-main-dashboard') {
          initBoardsMap(dealDefaultBoardsHook.trigger(defaultBoardsMap));
          success();
        }

        if (data.name == 'record-before-quit') {
          saveBoardsMap();
        }
      }
    });

    const handleAddBoardFromEmpty = () => {
      addBoardModalVisible.value = true;
      addBoardTargetBoardId.value = 0;
    };

    onBeforeUnmount(() => {
      subscription.unsubscribe();
      saveBoardsMap();
    });

    return {
      simpleImage,
      boardsMap,
      addBoardModalVisible,
      addBoardTargetBoardId,
      handleAddBoardFromEmpty,
    };
  },
});
</script>

<style lang="less">
.kf-index__warp {
  height: 100%;
  width: 100%;

  .kf-index__empty {
    margin: 25% auto;
  }

  & > .kf-drag-row__warp {
    height: 100%;
  }
}
</style>
