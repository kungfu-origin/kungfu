<template>
    <div class="kf-index__warp">
        <KfLayout>
            <KfRowColIter :board-id="0"></KfRowColIter>
        </KfLayout>
    </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

import KfLayout from '@renderer/components/layout/KfLayout.vue';
import KfRowColIter from '@renderer/components/layout/KfRowColIter.vue';

import { useGlobalStore } from '@renderer/pages/index/store/global';
import { defaultBoardsMap, getBoardsMap } from '@renderer/assets/configs';

export default defineComponent({
    name: 'Index',

    setup() {
        const boardsMap: KfLayout.BoardsMap =
            getBoardsMap() || defaultBoardsMap;

        const store = useGlobalStore();
        store.initBoardsMap(boardsMap);
    },

    components: {
        KfLayout,
        KfRowColIter,
    },
});
</script>

<style lang="less">
.kf-index__warp {
    height: 100%;
    width: 100%;

    .ant-layout-content > .kf-drag-row__warp {
        height: 100%;
    }
}
</style>
