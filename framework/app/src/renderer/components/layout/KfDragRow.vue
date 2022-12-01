<template>
  <div
    ref="kf-drag-row"
    :class="'kf-drag-row__warp'"
    :style="style"
    :board-id="id"
  >
    <div
      class="kf-drag-row__content"
      @mousedown="handleMouseDown"
      @mousemove="handleMouseMove"
      @mouseup="handleMouseUp"
    >
      <slot></slot>
    </div>
    <div v-if="id !== 0" class="resize-bar-horizontal"></div>
  </div>
</template>

<script lang="ts">
import { defineComponent, PropType, reactive, toRefs } from 'vue';
import { storeToRefs } from 'pinia';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';

interface KfDragRowData {
  resizeing: boolean;
  isOutOfBoardListenerAdded: boolean;
  leftCol$: HTMLElement | null;
  leftBoardId: string;
  leftColWidth: number;
  rightCol$: HTMLElement | null;
  rightBoardId: string;
  rightColWidth: number;
  paBoundingRect: DOMRect | { [x: string]: number };
  preX: number;
}

export default defineComponent({
  name: 'KfDragRow',

  props: {
    id: {
      required: true,
      type: Number as PropType<number>,
    },
  },

  setup() {
    const rowData = reactive<KfDragRowData>({
      resizeing: false,
      isOutOfBoardListenerAdded: false,
      leftCol$: null,
      leftBoardId: '',
      leftColWidth: 0,
      rightCol$: null,
      rightBoardId: '',
      rightColWidth: 0,
      paBoundingRect: {},
      preX: 0,
    });

    const { boardsMap } = storeToRefs(useGlobalStore());
    const { setBoardsMapAttrById } = useGlobalStore();

    return {
      ...toRefs(rowData),

      boardsMap,
      setBoardsMapAttrById,
    };
  },

  computed: {
    boardInfo(): KfLayout.BoardInfo {
      return this.boardsMap[this.id];
    },

    style(): string {
      if (this.boardInfo?.height) {
        if (this.boardInfo?.height.toString().includes('%')) {
          return `height: ${this.boardInfo?.height}; flex: unset;`;
        } else {
          return `height: ${this.boardInfo?.height}px; flex: unset;`;
        }
      } else {
        return ``;
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

      if (target.className === 'resize-bar-vertical') {
        this.leftCol$ = target.parentElement;
        this.leftBoardId = this.leftCol$?.getAttribute('board-id') || '';
        this.leftColWidth = this.leftCol$?.clientWidth || 0;
        this.rightCol$ = target.parentElement?.nextSibling as HTMLElement;
        this.rightBoardId = this.rightCol$?.getAttribute('board-id') || '';
        this.rightColWidth = this.rightCol$?.clientWidth || 0;
        const paElement = this.leftCol$?.parentElement;
        if (paElement) {
          this.paBoundingRect = paElement.getBoundingClientRect();
        }
        this.preX = e.x;
        this.resizeing = true;
      }
    },

    handleMouseMove(e: MouseEvent) {
      if (!this.resizeing) return;

      const currentY: number = e.y;
      const currentX: number = e.x;
      const deltaX = currentX - this.preX;

      if (
        this.paBoundingRect &&
        this.isMouseOutOfParent(currentX, currentY) &&
        !this.isOutOfBoardListenerAdded
      ) {
        document.addEventListener('mouseup', this.handleMouseUp);
        this.isOutOfBoardListenerAdded = true;
      }

      if (
        !this.leftCol$ ||
        !this.rightCol$ ||
        !this.leftBoardId ||
        !this.rightBoardId
      ) {
        return;
      }

      if (this.leftColWidth + deltaX <= 16 || this.rightColWidth - deltaX <= 16)
        return;

      this.leftColWidth += deltaX;
      this.rightColWidth -= deltaX;
      this.leftCol$.style.width = this.leftColWidth + 'px';
      this.leftCol$.style.flex = 'unset';
      this.rightCol$.style.width = this.rightColWidth + 'px';
      this.rightCol$.style.flex = 'unset';
      this.preX = currentX;
    },

    handleMouseUp() {
      if (
        !this.leftCol$ ||
        !this.rightCol$ ||
        !this.leftBoardId ||
        !this.rightBoardId
      ) {
        this.clearState();
        return;
      }

      //to percent for prevention of window resize
      this.setBoardsMapAttrById(
        +this.leftBoardId,
        'width',
        Number((this.leftColWidth * 100) / this.paBoundingRect.width).toFixed(
          3,
        ) + '%',
      );
      this.setBoardsMapAttrById(
        +this.rightBoardId,
        'width',
        Number((this.rightColWidth * 100) / this.paBoundingRect.width).toFixed(
          3,
        ) + '%',
      );

      this.$globalBus.next({
        tag: 'resize',
      } as KfEvent.ResizeEvent);

      this.clearState();
    },

    clearState() {
      this.resizeing = false;
      this.isOutOfBoardListenerAdded = false;
      this.leftCol$ = null;
      this.leftColWidth = 0;
      this.rightCol$ = null;
      this.rightColWidth = 0;
      this.preX = 0;
      document.removeEventListener('mouseup', this.handleMouseUp);
    },
  },
});
</script>

<style lang="less">
.kf-drag-row__warp {
  width: 100%;
  flex: 1;
  position: relative;
  transform: translateZ(0);
  overflow: hidden;

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
    height: 8px;
    width: 100%;
    border-bottom: 4px solid #000;
    cursor: row-resize;
    box-sizing: border-box;
    z-index: 10;

    &:hover {
      border-bottom: 4px solid @border-color-split;
    }
  }
}
</style>
