<template>
  <div class="kf-drag-col__warp" :style="style" :board-id="id">
    <div
      class="kf-drag-col__content"
      @mousedown="handleMouseDown"
      @mousemove="handleMouseMove"
      @mouseup="handleMouseUp"
    >
      <slot></slot>
    </div>
    <div class="resize-bar-vertical"></div>
  </div>
</template>

<script lang="ts">
import { defineComponent, PropType, reactive, toRefs } from 'vue';
import { storeToRefs } from 'pinia';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';

export default defineComponent({
  name: 'KfDragCol',

  props: {
    id: {
      required: true,
      type: Number as PropType<number>,
    },
  },

  setup() {
    const colData = reactive<{
      resizeing: boolean;
      isOutOfBoardListenerAdded: boolean;
      upRow$: HTMLElement | null;
      upBoardId: string;
      upRowHeight: number;
      bottomRow$: HTMLElement | null;
      bottomBoardId: string;
      bottomRowHeight: number;
      paBoundingRect: DOMRect | { [x: string]: number };
      preY: number;
    }>({
      resizeing: false,
      isOutOfBoardListenerAdded: false,
      upRow$: null,
      upBoardId: '',
      upRowHeight: 0,
      bottomRow$: null,
      bottomBoardId: '',
      bottomRowHeight: 0,
      paBoundingRect: {},
      preY: 0,
    });

    const { boardsMap } = storeToRefs(useGlobalStore());
    const { setBoardsMapAttrById } = useGlobalStore();

    return {
      ...toRefs(colData),
      boardsMap,
      setBoardsMapAttrById,
    };
  },

  computed: {
    boardInfo(): KfLayout.BoardInfo {
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
    isMouseOutOfParent(currentX: number, currentY: number) {
      return (
        currentX <= this.paBoundingRect.left + 16 ||
        currentX >= this.paBoundingRect.right - 16 ||
        currentY <= this.paBoundingRect.top + 16 ||
        currentY >= this.paBoundingRect.bottom - 16
      );
    },

    handleMouseDown(e: MouseEvent) {
      const target = e.target as HTMLElement;

      if (target.className === 'resize-bar-horizontal') {
        this.upRow$ = target.parentElement;
        this.upBoardId = this.upRow$?.getAttribute('board-id') || '';
        this.upRowHeight = this.upRow$?.clientHeight || 0;
        this.bottomRow$ = target.parentElement?.nextSibling as HTMLElement;
        this.bottomBoardId = this.bottomRow$?.getAttribute('board-id') || '';
        this.bottomRowHeight = this.bottomRow$?.clientHeight || 0;
        const paElement = this.upRow$?.parentElement;
        if (paElement) {
          this.paBoundingRect = paElement.getBoundingClientRect();
        }
        this.preY = e.y;
        this.resizeing = true;
      }
    },

    handleMouseMove(e: MouseEvent) {
      if (!this.resizeing) return;

      const currentY: number = e.y;
      const currentX: number = e.x;
      const deltaY = currentY - this.preY;

      if (
        this.paBoundingRect &&
        this.isMouseOutOfParent(currentX, currentY) &&
        !this.isOutOfBoardListenerAdded
      ) {
        document.addEventListener('mouseup', this.handleMouseUp);
        this.isOutOfBoardListenerAdded = true;
      }

      if (
        !this.upRow$ ||
        !this.bottomRow$ ||
        !this.upBoardId ||
        !this.bottomBoardId
      ) {
        return;
      }

      if (
        this.upRowHeight + deltaY <= 16 ||
        this.bottomRowHeight - deltaY <= 16
      )
        return;

      this.upRowHeight += deltaY;
      this.bottomRowHeight -= deltaY;
      this.upRow$.style.height = this.upRowHeight + 'px';
      this.upRow$.style.flex = 'unset';
      this.bottomRow$.style.height = this.bottomRowHeight + 'px';
      this.bottomRow$.style.flex = 'unset';
      this.preY = currentY;
    },

    handleMouseUp() {
      if (
        !this.upRow$ ||
        !this.bottomRow$ ||
        !this.upBoardId ||
        !this.bottomBoardId ||
        !this.resizeing
      ) {
        this.clearState();
        return;
      }

      this.setBoardsMapAttrById(
        +this.upBoardId,
        'height',
        this.paBoundingRect.height
          ? Number(
              (this.upRowHeight * 100) / this.paBoundingRect.height,
            ).toFixed(3) + '%'
          : this.upRowHeight,
      );
      this.setBoardsMapAttrById(
        +this.bottomBoardId,
        'height',
        this.paBoundingRect.height
          ? Number(
              (this.bottomRowHeight * 100) / this.paBoundingRect.height,
            ).toFixed(3) + '%'
          : 0,
      );

      this.$globalBus.next({
        tag: 'resize',
      } as KfEvent.ResizeEvent);

      this.clearState();
    },

    clearState() {
      this.resizeing = false;
      this.isOutOfBoardListenerAdded = false;
      this.upRow$ = null;
      this.upRowHeight = 0;
      this.bottomRow$ = null;
      this.bottomRowHeight = 0;
      this.paBoundingRect = {};
      this.preY = 0;
      document.removeEventListener('mouseup', this.handleMouseUp);
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
    width: 8px;
    border-right: 4px solid #000;
    cursor: col-resize;
    box-sizing: border-box;
    z-index: 10;

    &:hover {
      border-right: 4px solid @border-color-split;
    }
  }
}
</style>
