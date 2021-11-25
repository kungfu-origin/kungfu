<template>
    <div class="kf-drag-col__warp" :style="style" :board-id="id">
        <div
            class="kf-drag-col__content"
            @mousedown="handleMouseDown"
            @mousemove="handleMouseMove"
            @mouseup="hanleMouseUp"
        >
            <slot></slot>
        </div>
        <div class="resize-bar-vertical"></div>
    </div>
</template>

<script lang="ts">
import { defineComponent, PropType } from '@vue/runtime-core';
import { mapActions, mapState } from 'pinia';

import { useGlobalStore } from '@renderer/index/store/global';
import { BoardInfo } from '@renderer/components/global/types/index';

interface KfDragColData {
    $upRow: HTMLElement | undefined;
    upBoardId: string;
    upRowHeight: number;
    $bottomRow: HTMLElement | undefined;
    bottomBoardId: string;
    bottomRowHeight: number;
    paHeight: number;
    preY: number;
}

export default defineComponent({
    name: 'KfDragCol',

    props: {
        id: {
            required: true,
            type: Number as PropType<number>,
        },
    },

    data(): KfDragColData {
        return {
            $upRow: undefined,
            upBoardId: '',
            upRowHeight: 0,
            $bottomRow: undefined,
            bottomBoardId: '',
            bottomRowHeight: 0,
            paHeight: 0,
            preY: 0,
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
            if (this.boardInfo?.width) {
                if (this.boardInfo?.width.toString().includes('%')) {
                    return `width: ${this.boardInfo?.width}; flex: unset;`;
                } else {
                    return `width: ${this.boardInfo?.width}px; flex: unset;`;
                }
            } else {
                return `flex: 1;`;
            }
        },
    },

    methods: {
        ...mapActions(useGlobalStore, {
            setBoardsMapAttrById: 'setBoardsMapAttrById',
        }),

        handleMouseDown(e: MouseEvent) {
            const target = e.target as HTMLElement;

            if (target.className === 'resize-bar-horizontal') {
                this.$upRow = target.parentNode as HTMLElement;
                this.upBoardId = this.$upRow.getAttribute('board-id') || '';
                this.upRowHeight = this.$upRow?.clientHeight || 0;
                this.$bottomRow = target.parentNode?.nextSibling as HTMLElement;
                this.bottomBoardId =
                    this.$bottomRow.getAttribute('board-id') || '';
                this.bottomRowHeight = this.$bottomRow?.clientHeight || 0;
                this.paHeight = this.$upRow.parentNode.clientHeight;
                this.preY = e.y;
            }
        },

        handleMouseMove(e: MouseEvent) {
            const currentY: number = e.y;
            const deltaY = currentY - this.preY;

            if (
                !this.$upRow ||
                !this.$bottomRow ||
                !this.upBoardId ||
                !this.bottomBoardId
            ) {
                return;
            }

            this.upRowHeight += deltaY;
            this.bottomRowHeight -= deltaY;
            this.$upRow.style.height = this.upRowHeight + 'px';
            this.$upRow.style.flex = 'unset';
            this.$bottomRow.style.height = this.bottomRowHeight + 'px';
            this.$bottomRow.style.flex = 'unset';
            this.preY = currentY;
        },

        hanleMouseUp() {
            if (
                !this.$upRow ||
                !this.$bottomRow ||
                !this.upBoardId ||
                !this.bottomBoardId
            ) {
                return;
            }

            this.setBoardsMapAttrById(
                +this.upBoardId,
                'height',
                Number((this.upRowHeight * 100) / this.paHeight).toFixed(3) +
                    '%',
            );
            this.setBoardsMapAttrById(
                +this.bottomBoardId,
                'height',
                Number((this.bottomRowHeight * 100) / this.paHeight).toFixed(
                    3,
                ) + '%',
            );

            this.$upRow = undefined;
            this.upRowHeight = 0;
            this.$bottomRow = undefined;
            this.bottomRowHeight = 0;
            this.preY = 0;
        },
    },
});
</script>

<style lang="less">
.kf-drag-col__warp {
    height: 100%;
    flex: 1;
    position: relative;
    transform: translateZ(0);
    overflow: hidden;

    .kf-drag-col__content {
        display: flex;
        flex-direction: column;
        height: 100%;
        width: 100%;
        justify-content: flex-start;

        > .kf-drag-row__warp:last-of-type {
            > .resize-bar-horizontal {
                display: none;
            }
        }
    }

    .resize-bar-vertical {
        position: absolute;
        right: 0;
        top: 0;
        height: 100%;
        width: 5px;
        border-right: 8px solid #000;
        cursor: col-resize;
        box-sizing: border-box;
        z-index: 10;

        &:hover {
            border-right: 8px solid @border-color-base;
        }
    }
}
</style>
