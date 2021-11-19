<template>
    <div class="kf-drag-row__warp">
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
import { defineComponent } from '@vue/runtime-core';

const $body = document.body;
export default defineComponent({
    name: 'KfDragRow',

    data() {
        return {
            $leftCol: $body,
            leftColWidth: 0,
            $rightCol: $body,
            rightColWidth: 0,
            preX: 0,
        };
    },

    methods: {
        handleMouseDown(e: MouseEvent) {
            const target = e.target as HTMLBaseElement;

            if (target.className === 'resize-bar-vertical') {
                this.$leftCol = target.parentNode as HTMLBaseElement;
                this.leftColWidth = this.$leftCol?.clientWidth || 0;
                this.$rightCol = target.parentNode
                    ?.nextSibling as HTMLBaseElement;
                this.rightColWidth = this.$rightCol?.clientWidth || 0;
                this.preX = e.x;
            }
        },

        handleMouseMove(e: MouseEvent) {
            const currentX: number = e.x;
            const deltaX = currentX - this.preX;

            if (!this.$leftCol || !this.$rightCol) {
                return;
            }

            if (this.$leftCol !== $body && this.$rightCol !== $body) {
                this.leftColWidth += deltaX;
                this.rightColWidth -= deltaX;
                this.$leftCol.style.width = this.leftColWidth + 'px';
                this.$leftCol.style.flex = 'unset';
                this.$rightCol.style.width = this.rightColWidth + 'px';
                this.$rightCol.style.flex = 'unset';
                this.preX = currentX;
            }
        },

        hanleMouseUp() {
            this.$leftCol = $body;
            this.leftColWidth = 0;
            this.$rightCol = $body;
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

        > .kf-drag-col__warp:last-child {
            .resize-bar-vertical {
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
