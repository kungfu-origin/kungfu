<template>
    <KfDragRow v-if="boardInfo.direction === 'h'">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter
                :board-id="childBoardId"
                :boards-map="boardsMap"
            ></KfRowColIter>
        </template>
        <template v-if="boardInfo.contents">
            <a-card
                size="small"
                style="height: 100%; width: 100%"
                :bordered="false"
                :tab-list="boardInfo.contents.map((item: string) => ({key: item, tab: item}))"
            >
            {{boardInfo.current || ""}}
            </a-card>
        </template>
    </KfDragRow>
    <KfDragCol v-else-if="boardInfo.direction === 'v'">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter
                :board-id="childBoardId"
                :boards-map="boardsMap"
            ></KfRowColIter>
        </template>
        <template v-if="boardInfo.contents">
            <a-card
                size="small"
                style="height: 100%; width: 100%"
                :bordered="false"
                :tab-list="boardInfo.contents.map((item: string) => ({key: item, tab: item}))"
            >
                {{boardInfo.current || ""}}
            </a-card>
        </template>
    </KfDragCol>
</template>

<script lang="ts">
import { defineComponent, PropType } from '@vue/runtime-core';

import KfDragRow from '@/components/global/KfDragRow.vue';
import KfDragCol from '@/components/global/KfDragCol.vue';

import { BoardInfo, BoradsMap } from '@/components/global/typings/index';

export default defineComponent({
    name: 'KfRowColIter',

    components: {
        KfDragCol,
        KfDragRow,
    },

    props: {
        boardId: {
            type: Number,
            default: 0,
        },

        boardsMap: {
            type: Object as PropType<BoradsMap>,
            default: () => ({ direction: 'h' } as BoradsMap),
        },
    },

    computed: {
        boardInfo(): BoardInfo {
            return this.boardsMap[this.boardId];
        },

        children(): number[] {
            return this.boardInfo.children || [];
        },

        contents(): string[] {
            return this.boardInfo.contents || [];
        },
    },
});
</script>
<style lang="less"></style>
