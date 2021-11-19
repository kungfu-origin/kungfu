<template>
    <div class="kf-drag-col__warp">
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
import { defineComponent } from '@vue/runtime-core';

const $body = document.body;
export default defineComponent({
    name: 'KfDragCol',

    data() {
        return {
            $upRow: $body,
            upRowHeight: 0,
            $bottomRow: $body,
            bottomRowHeight: 0,
            preY: 0,
        };
    },

    methods: {
        handleMouseDown(e: MouseEvent) {
            const target = e.target as HTMLBaseElement;

            if (target.className === 'resize-bar-horizontal') {
                this.$upRow = target.parentNode as HTMLBaseElement;
                this.upRowHeight = this.$upRow?.clientHeight || 0;
                this.$bottomRow = target.parentNode
                    ?.nextSibling as HTMLBaseElement;
                this.bottomRowHeight = this.$bottomRow?.clientHeight || 0;
                this.preY = e.y;
            }
        },

        handleMouseMove(e: MouseEvent) {
            const currentY: number = e.y;
            const deltaY = currentY - this.preY;

            if (!this.$upRow || !this.$bottomRow) {
                return;
            }

            if (this.$upRow != $body && this.$bottomRow != $body) {
                this.upRowHeight += deltaY;
                this.bottomRowHeight -= deltaY;
                this.$upRow.style.height = this.upRowHeight + 'px';
                this.$upRow.style.flex = 'unset';
                this.$bottomRow.style.height = this.bottomRowHeight + 'px';
                this.$bottomRow.style.flex = 'unset';
                this.preY = currentY;
            }
        },

        hanleMouseUp() {
            this.$upRow = $body;
            this.upRowHeight = 0;
            this.$bottomRow = $body;
            this.bottomRowHeight = 0;
            this.preY = 0;
        },
    },
});
</script>

<style lang="less">
.kf-drag-col__warp {
    height: 100%;
    width: 100%;
    flex: 1;
    position: relative;
    transform: translateZ(0);

    .kf-drag-col__content {
        display: flex;
        flex-direction: column;
        height: 100%;
        width: 100%;
        justify-content: flex-start;

        > .kf-drag-row__warp:last-child {
            .resize-bar-horizontal {
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
        border-right: 1px solid @border-color-base;
        cursor: col-resize;
        box-sizing: border-box;

        &:hover {
            right: -2px;
            border-right: 5px solid @border-color-base;
        }
    }
}
</style>
