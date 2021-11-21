<template>
    <div class="kf-index__warp">
        <KfLayout>
            <KfRowColIter :board-id="0"></KfRowColIter>
        </KfLayout>
    </div>
</template>

<script lang="ts">
import { defineComponent } from '@vue/runtime-core';

import KfLayout from '@/components/global/KfLayout.vue';
import KfRowColIter from '@/components/global/KfRowColIter.vue';

import { BoardsMap } from '@/components/global/typings/index';
import { useGlobalStore } from '@/store/global';

export default defineComponent({
    name: 'Index',

    setup() {
        const boardsMap: BoardsMap = {
            0: {
                direction: 'h',
                children: [1, 2, 3],
            },
            1: {
                direction: 'v',
                children: [4, 5],
                width: 400,
            },
            2: {
                direction: 'v',
                children: [8, 9],
            },
            3: {
                direction: 'v',
                children: [10, 11],
            },
            4: {
                direction: 'h',
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
                direction: 'h',
                children: [6, 7],
            },
            6: {
                direction: 'v',
                contents: ['6', '7'],
                current: '6',
            },
            7: {
                direction: 'v',
                contents: ['7', '8'],
                current: '8',
            },
            8: {
                direction: 'h',
                contents: ['8', '9'],
                current: '9',
                height: 300,
            },
            9: {
                direction: 'h',
                contents: ['9'],
                current: '9',
            },
            10: {
                direction: 'h',
                contents: ['10'],
                current: '10',
            },
            11: {
                direction: 'h',
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
}
</style>
