<template>
    <div class="kf-drag-row__warp" :style="style" :board-id="id">
        <div
            class="kf-drag-row__content"
            @mousedown="handleMouseDown"
            @mousemove="handleMouseMove"
            @mouseup="hanleMouseUp"
        >
            <slot></slot>
        </div>
        <div class="resize-bar-horizontal"></div>
    </div>
</template>

<script lang="ts">
import { defineComponent, PropType } from '@vue/runtime-core';
import { mapActions, mapState } from 'pinia';

import { useGlobalStore } from '@/store/global';
import { BoardInfo } from '@/components/global/typings';

export default defineComponent({
    name: 'KfDragRow',

    props: {
        id: {
            required: true,
            type: Number as PropType<number>,
        },
    },

    data() {
        return {
            $leftCol: undefined,
            leftBoardId: '',
            leftColWidth: 0,
            $rightCol: undefined,
            rightBoardId: '',
            rightColWidth: 0,
            preX: 0,
        } as {
            $leftCol: HTMLElement | undefined;
            leftBoardId: string;
            leftColWidth: number;
            $rightCol: HTMLElement | undefined;
            rightBoardId: string;
            rightColWidth: number;
            preX: number;
        };
    },

    computed: {
        ...mapState(useGlobalStore, {
            boardsMap: (store) => store.boardsMap,
        }),

        boardInfo(): BoardInfo {
            return this.boardsMap[this.id];
        },

        style(): string {
            if (this.boardInfo?.height) {
                return `height: ${this.boardInfo?.height}px; flex: unset;`;
            } else {
                return ``;
            }
        },
    },

    methods: {
        ...mapActions(useGlobalStore, {
            setBoardsMapAttrById: 'setBoardsMapAttrById',
        }),

        handleMouseDown(e: MouseEvent) {
            const target = e.target as HTMLElement;

            if (target.className === 'resize-bar-vertical') {
                this.$leftCol = target.parentNode as HTMLElement;
                this.leftBoardId = this.$leftCol.getAttribute('board-id') || '';
                this.leftColWidth = this.$leftCol?.clientWidth || 0;
                this.$rightCol = target.parentNode?.nextSibling as HTMLElement;
                this.rightBoardId =
                    this.$rightCol.getAttribute('board-id') || '';
                this.rightColWidth = this.$rightCol?.clientWidth || 0;
                this.preX = e.x;
            }
        },

        handleMouseMove(e: MouseEvent) {
            const currentX: number = e.x;
            const deltaX = currentX - this.preX;

            if (
                !this.$leftCol ||
                !this.$rightCol ||
                !this.leftBoardId ||
                !this.rightBoardId
            ) {
                return;
            }

            this.leftColWidth += deltaX;
            this.rightColWidth -= deltaX;
            this.$leftCol.style.width = this.leftColWidth + 'px';
            this.$leftCol.style.flex = 'unset';
            this.$rightCol.style.width = this.rightColWidth + 'px';
            this.$rightCol.style.flex = 'unset';
            this.preX = currentX;
        },

        hanleMouseUp() {
            if (
                !this.$leftCol ||
                !this.$rightCol ||
                !this.leftBoardId ||
                !this.rightBoardId
            ) {
                return;
            }

            this.setBoardsMapAttrById(
                +this.leftBoardId,
                'width',
                this.leftColWidth,
            );
            this.setBoardsMapAttrById(
                +this.rightBoardId,
                'width',
                this.rightColWidth,
            );

            this.$leftCol = undefined;
            this.leftColWidth = 0;
            this.$rightCol = undefined;
            this.rightColWidth = 0;
            this.preX = 0;
        },
    },
});
</script>

<style lang="less">
.kf-drag-row__warp {
    height: 100%;
    width: 100%;
    flex: 1;
    position: relative;
    transform: translateZ(0);

    .kf-drag-row__content {
        display: flex;
        flex-direction: row;
        height: 100%;
        width: 100%;
        justify-content: flex-start;

        > .kf-drag-col__warp:last-of-type {
            > .resize-bar-vertical {
                display: none;
            }
        }
    }

    .resize-bar-horizontal {
        position: absolute;
        left: 0;
        bottom: 0;
        height: 5px;
        width: 100%;
        border-bottom: 1px solid @border-color-base;
        cursor: row-resize;
        box-sizing: border-box;
        z-index: 10;

        &:hover {
            border-bottom: 5px solid @border-color-base;
        }
    }
}
</style>
