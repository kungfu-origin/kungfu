<template>
  <div class="kf-index__warp">
    <KfRowColIter :board-id="0"></KfRowColIter>
  </div>
</template>

<script lang="ts">
import { defineComponent, onBeforeUnmount } from 'vue';

import KfRowColIter from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfRowColIter.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import {
  defaultBoardsMap,
  getIndexBoardsMap,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/configs';
import { usePreStartAndQuitApp } from '../../../assets/methods/actionsUtils';

export default defineComponent({
  name: 'Index',

  setup() {
    const boardsMap: KfLayout.BoardsMap =
      getIndexBoardsMap() || defaultBoardsMap;

    const store = useGlobalStore();
    store.initBoardsMap(boardsMap);

    const { saveBoardsMap } = usePreStartAndQuitApp();
    onBeforeUnmount(() => {
      saveBoardsMap();
    });
  },

  components: {
    KfRowColIter,
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
