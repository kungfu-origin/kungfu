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

import { KfLayoutDirection } from '@root/src/typings/enums';
import { useGlobalStore } from '@renderer/pages/index/store/global';

export default defineComponent({
    name: 'Index',

    setup() {
        const boardsMap: KfLayout.BoardsMap = {
            0: {
                paId: -1,
                direction: KfLayoutDirection.h,
                children: [1, 2],
            },
            1: {
                paId: 0,
                direction: KfLayoutDirection.v,
                children: [4, 5],
                width: '60%',
            },
            2: {
                paId: 0,
                direction: KfLayoutDirection.v,
                children: [8, 9],
            },
            4: {
                paId: 1,
                direction: KfLayoutDirection.h,
                contents: ['交易账户'],
                current: '交易账户',
            },
            5: {
                paId: 1,
                direction: KfLayoutDirection.h,
                children: [6],
            },
            6: {
                paId: 5,
                direction: KfLayoutDirection.v,
                contents: ['策略进程'],
                current: '策略进程',
            },
            8: {
                paId: 2,
                direction: KfLayoutDirection.h,
                contents: ['8', '行情源'],
                current: '行情源',
            },
            9: {
                paId: 2,
                direction: KfLayoutDirection.h,
                contents: ['9'],
                current: '9',
            },
        };

        useGlobalStore().initBoardsMap(boardsMap);
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
