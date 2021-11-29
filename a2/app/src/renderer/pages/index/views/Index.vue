<template>
    <div class="kf-index__warp">
        <KfLayout>
            <KfRowColIter :board-id="0"></KfRowColIter>
        </KfLayout>
    </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

import KfLayout from '@renderer/components/global/KfLayout.vue';
import KfRowColIter from '@renderer/components/global/KfRowColIter.vue';

import { BoardsMap, Direction } from '@renderer/types/index';
import { useGlobalStore } from '@renderer/pages/index/store/global';

export default defineComponent({
    name: 'Index',

    setup() {
        const boardsMap: BoardsMap = {
            0: {
                paId: -1,
                direction: Direction.h,
                children: [1, 2],
            },
            1: {
                paId: 0,
                direction: Direction.v,
                children: [4, 5],
                width: '60%',
            },
            2: {
                paId: 0,
                direction: Direction.v,
                children: [8, 9, 10],
            },
            4: {
                paId: 1,
                direction: Direction.h,
                contents: ['交易账户', '行情源'],
                current: '4',
            },
            5: {
                paId: 1,
                direction: Direction.h,
                children: [6],
            },
            6: {
                paId: 5,
                direction: Direction.v,
                contents: ['策略进程'],
                current: '6',
            },
            8: {
                paId: 2,
                direction: Direction.h,
                contents: ['8', '9'],
                current: '9',
            },
            9: {
                paId: 2,
                direction: Direction.h,
                contents: ['9'],
                current: '9',
            },
            10: {
                paId: 2,
                direction: Direction.h,
                contents: ['test3'],
                current: '10',
            },
        };

        const globalStore = useGlobalStore();
        globalStore.initBoardsMap(boardsMap);
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
