<template>
    <div class="kf-index__warp">
        <KfLayout>
            <KfRowColIter :board-id="0"></KfRowColIter>
        </KfLayout>
    </div>
</template>

<script lang="ts">
import { defineComponent } from '@vue/runtime-core';

import KfLayout from '@/index/components/global/KfLayout.vue';
import KfRowColIter from '@/index/components/global/KfRowColIter.vue';

import { BoardsMap, Direction } from '@/index/components/global/typings/index';
import { useGlobalStore } from '@/index/store/global';

export default defineComponent({
    name: 'Index',

    setup() {
        const boardsMap: BoardsMap = {
            0: {
                paId: -1,
                direction: Direction.h,
                children: [1, 2, 3],
            },
            1: {
                paId: 0,
                direction: Direction.v,
                children: [4, 5],
            },
            2: {
                paId: 0,
                direction: Direction.v,
                children: [8, 9],
            },
            3: {
                paId: 0,
                direction: Direction.v,
                children: [10, 11],
            },
            4: {
                paId: 1,
                direction: Direction.h,
                contents: [
                    '4',
                    '5',
                    '6',
                    '7',
                    '8',
                    '9',
                    '14',
                    '15',
                    '16',
                    '17',
                    '18',
                    '19',
                    '20',
                    '21',
                ],
                current: '4',
            },
            5: {
                paId: 1,
                direction: Direction.h,
                children: [6, 7],
            },
            6: {
                paId: 5,
                direction: Direction.v,
                contents: ['6', '账户进程'],
                current: '6',
            },
            7: {
                paId: 5,
                direction: Direction.v,
                contents: ['7', '阿斯顿发空间', '10'],
                current: '7',
            },
            8: {
                paId: 2,
                direction: Direction.h,
                contents: ['8', '9'],
                current: '9',
                height: 300,
            },
            9: {
                paId: 2,
                direction: Direction.h,
                contents: ['9'],
                current: '9',
            },
            10: {
                paId: 3,
                direction: Direction.h,
                contents: ['10'],
                current: '10',
            },
            11: {
                paId: 3,
                direction: Direction.h,
                contents: ['11'],
                current: '11',
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
