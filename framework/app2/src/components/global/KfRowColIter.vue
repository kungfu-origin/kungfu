<template>
    <KfDragRow :id="boardId" v-if="direction === 'h'">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter :board-id="childBoardId"></KfRowColIter>
        </template>
        <template v-if="boardInfo.contents">
            <a-tabs
                size="small"
                :activeKey="boardInfo.current"
                style="height: 100%; width: 100%"
                type="editable-card"
                @edit="hanldeEdit"
            >
                <a-tab-pane
                    v-for="content in contents"
                    :key="content"
                    :tab="content"
                >
                    {{ content }}
                </a-tab-pane>
            </a-tabs>
        </template>
    </KfDragRow>
    <KfDragCol :id="boardId" v-else-if="direction === 'v'">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter :board-id="childBoardId"></KfRowColIter>
        </template>
        <template v-if="boardInfo.contents">
            <a-tabs
                size="small"
                :activeKey="boardInfo.current"
                style="height: 100%; width: 100%"
                type="editable-card"
                @edit="hanldeEdit"
            >
                <a-tab-pane
                    v-for="content in contents"
                    :key="content"
                    :tab="content"
                >
                    {{ content }}
                </a-tab-pane>
            </a-tabs>
        </template>
    </KfDragCol>
</template>

<script lang="ts">
import { defineComponent, PropType } from '@vue/runtime-core';
import { mapState } from 'pinia';

import KfDragRow from '@/components/global/KfDragRow.vue';
import KfDragCol from '@/components/global/KfDragCol.vue';

import { BoardInfo, BoardsMap } from '@/components/global/typings/index';
import { useGlobalStore } from '@/store/global';

export default defineComponent({
    name: 'KfRowColIter',

    components: {
        KfDragCol,
        KfDragRow,
    },

    props: {
        boardId: {
            type: Number as PropType<number>,
            default: 0,
        },
    },

    computed: {
        ...mapState(useGlobalStore, {
            boardsMap: (store): BoardsMap => store.boardsMap,
        }),

        boardInfo(): BoardInfo {
            return this.boardsMap[this.boardId];
        },

        children(): number[] {
            return this.boardInfo.children || [];
        },

        contents(): string[] {
            return this.boardInfo.contents || [];
        },

        direction(): string {
            return this.boardInfo.direction;
        },
    },

    methods: {
        hanldeEdit(e: MouseEvent) {
            console.log(e);
        },
    },
});
</script>
<style lang="less"></style>
