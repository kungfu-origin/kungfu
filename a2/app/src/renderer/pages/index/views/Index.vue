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
                children: [4, 5, 6],
                width: '60%',
            },
            2: {
                paId: 0,
                direction: KfLayoutDirection.v,
                children: [8, 9, 10],
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
                contents: ['策略进程'],
                current: '策略进程',
            },
            6: {
                paId: 1,
                direction: KfLayoutDirection.v,
                contents: ['委托记录', '成交记录'],
                current: '委托记录',
            },
            8: {
                paId: 2,
                direction: KfLayoutDirection.h,
                contents: ['行情源'],
                current: '行情源',
            },
            9: {
                paId: 2,
                direction: KfLayoutDirection.h,
                contents: ['持仓'],
                current: '持仓',
            },
            10: {
                paId: 2,
                direction: KfLayoutDirection.h,
                children: [11, 12],
            },
            11: {
                paId: 10,
                direction: KfLayoutDirection.v,
                contents: ['深度行情'],
                current: '深度行情',
            },
            12: {
                paId: 10,
                direction: KfLayoutDirection.v,
                contents: ['下单面板'],
                current: '下单面板',
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
