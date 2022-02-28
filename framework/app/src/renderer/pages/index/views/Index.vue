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
import {
  defineComponent,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
} from 'vue';

import KfRowColIter from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfRowColIter.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import {
  defaultBoardsMap,
  getIndexBoardsMap,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/configs';
import { usePreStartAndQuitApp } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import KfAddBoardModalVue from '../../../components/public/KfAddBoardModal.vue';
import { message } from 'ant-design-vue';

export default defineComponent({
  name: 'Index',

  setup() {
    const boardsMap: KfLayout.BoardsMap =
      getIndexBoardsMap() || defaultBoardsMap;

    const store = useGlobalStore();
    store.initBoardsMap(boardsMap);

    const addBoardModalVisible = ref<boolean>(false);
    const addBoardTargetBoardId = ref<number>(-1);

    const { saveBoardsMap } = usePreStartAndQuitApp();
    const app = getCurrentInstance();
    onMounted(() => {
      if (app?.proxy) {
        const subscription = app.proxy.$globalBus.subscribe(
          (data: KfBusEvent) => {
            if (data.tag === 'addBoard') {
              addBoardModalVisible.value = true;
              addBoardTargetBoardId.value = data.boardId;
            }

            if (data.tag === 'main') {
              if (data.name === 'reset-main-dashboard') {
                store.initBoardsMap(defaultBoardsMap);
                message.success('操作成功');
              }
            }
          },
        );

        onBeforeUnmount(() => {
          subscription.unsubscribe();
          saveBoardsMap();
        });
      }
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
