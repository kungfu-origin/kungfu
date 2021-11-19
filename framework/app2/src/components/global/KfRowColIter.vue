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
            <a-tabs
                size="small"
                v-model:current="boardInfo.current"
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
            <a-tabs
                size="small"
                v-model:current="boardInfo.current"
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

    methods: {
        hanldeEdit(e: MouseEvent) {
            console.log(e);
        },
    },
});
</script>
<style lang="less"></style>
