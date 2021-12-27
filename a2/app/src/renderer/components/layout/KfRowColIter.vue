<template>
    <KfDragRow :id="boardId" v-if="direction === h">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter :board-id="childBoardId"></KfRowColIter>
        </template>
        <template v-if="contents.length">
            <a-tabs
                size="small"
                :class="{
                    [classNameForTabDrag]: true,
                    'is-dragging': isBoardDragging,
                }"
                :activeKey="boardInfo.current"
                style="height: 100%; width: 100%"
                type="card"
                :tabBarStyle="{ margin: 0 }"
                @edit="hanldeEdit"
                @tabClick="handleClickTab"
                @dragenter="handleDragEnter"
                @dragover="handleDragOver"
                @dragleave="handleDragLeave"
                @dragend="handleDragEnd"
                @drop="handleDrop"
            >
                <a-tab-pane v-for="content in contents" :key="content">
                    <template v-slot:tab>
                        <div
                            class="kf-tab-header"
                            draggable="true"
                            @dragstart="handleDragStart(content)"
                        >
                            {{ content }}
                        </div>
                    </template>
                    <a-card style="width: 100%; height: 100%">
                        <component
                            v-if="
                                hasComponent(content) &&
                                content === boardInfo.current
                            "
                            :is="content"
                            :id="content"
                        ></component>
                        <KfNoData
                            v-else
                            :txt="`${content} 组件不存在`"
                        ></KfNoData>
                    </a-card>
                </a-tab-pane>
            </a-tabs>
        </template>
    </KfDragRow>
    <KfDragCol :id="boardId" v-else-if="direction === v">
        <template
            v-for="childBoardId in boardInfo.children || []"
            :key="childBoardId"
        >
            <KfRowColIter :board-id="childBoardId"></KfRowColIter>
        </template>
        <template v-if="contents.length">
            <a-tabs
                size="small"
                :class="{
                    [classNameForTabDrag]: true,
                    'is-dragging': isBoardDragging,
                }"
                :activeKey="boardInfo.current"
                style="height: 100%; width: 100%"
                type="card"
                :tabBarStyle="{ margin: 0 }"
                @edit="hanldeEdit"
                @tabClick="handleClickTab"
                @dragenter="handleDragEnter"
                @dragover="handleDragOver"
                @dragleave="handleDragLeave"
                @dragend="handleDragEnd"
                @drop="handleDrop"
            >
                <a-tab-pane v-for="content in contents" :key="content">
                    <template v-slot:tab>
                        <div
                            class="kf-tab-header"
                            draggable="true"
                            @dragstart="handleDragStart(content)"
                        >
                            {{ content }}
                        </div>
                    </template>
                    <a-card style="width: 100%; height: 100%">
                        <component
                            v-if="
                                hasComponent(content) &&
                                content === boardInfo.current
                            "
                            :is="content"
                            :id="content"
                        ></component>
                        <KfNoData
                            v-else
                            :txt="`${content} 组件不存在`"
                        ></KfNoData>
                    </a-card>
                </a-tab-pane>
            </a-tabs>
        </template>
    </KfDragCol>
</template>

<script lang="ts">
import { defineComponent, PropType, reactive, toRefs } from 'vue';
import { mapActions, mapState } from 'pinia';
import {
    KfLayoutDirection,
    KfLayoutTargetDirectionClassName,
} from '@root/src/typings/enums';

import KfDragRow from '@renderer/components/layout/KfDragRow.vue';
import KfDragCol from '@renderer/components/layout/KfDragCol.vue';
import KfNoData from '@renderer/components/public/KfNoData.vue';

import { useGlobalStore } from '@renderer/pages/index/store/global';

interface KfRowColIterData {
    h: KfLayoutDirection;
    v: KfLayoutDirection;
    classNameForTabDrag: KfLayoutTargetDirectionClassName;
    dragEnterBoxWidth14: number;
    dragEnterBoxWidth34: number;
    dragEnterBoxHeight14: number;
    dragEnterBoxHeight34: number;
}

export default defineComponent({
    name: 'KfRowColIter',

    components: {
        KfDragCol,
        KfDragRow,
        KfNoData,
    },

    props: {
        boardId: {
            type: Number as PropType<number>,
            default: 0,
        },
    },

    setup() {
        const rowColIterData = reactive<KfRowColIterData>({
            h: KfLayoutDirection.h,
            v: KfLayoutDirection.v,
            classNameForTabDrag: KfLayoutTargetDirectionClassName.unset,
            dragEnterBoxWidth14: 0,
            dragEnterBoxWidth34: 0,
            dragEnterBoxHeight14: 0,
            dragEnterBoxHeight34: 0,
        });

        return {
            ...toRefs(rowColIterData),
        };
    },

    computed: {
        ...mapState(useGlobalStore, {
            boardsMap: (store): KfLayout.BoardsMap => store.boardsMap,
            dragedContentData: (store): KfLayout.ContentData | null =>
                store.dragedContentData,
            isBoardDragging: (store): boolean => store.isBoardDragging,
        }),

        boardInfo(): KfLayout.BoardInfo {
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
        ...mapActions(useGlobalStore, {
            setBoardsMapAttrById: 'setBoardsMapAttrById',
            removeBoardByContentId: 'removeBoardByContentId',
            setDragedContentData: 'setDragedContentData',
            afterDragMoveBoard: 'afterDragMoveBoard',
            markIsBoardDragging: 'markIsBoardDragging',
        }),

        handleDragStart(contentId: KfLayout.ContentId) {
            this.setDragedContentData(this.boardId, contentId);
            this.markIsBoardDragging(true);
        },

        handleDragEnter(e: DragEvent) {
            const target: HTMLElement | null = e.target as HTMLElement | null;
            const width: number = target?.clientWidth || 0;
            const height: number = target?.clientHeight || 0;

            this.dragEnterBoxWidth14 = width / 4;
            this.dragEnterBoxWidth34 = (width * 3) / 4;
            this.dragEnterBoxHeight14 = height / 4;
            this.dragEnterBoxHeight34 = (height * 3) / 4;
        },

        handleDragOver(e: DragEvent) {
            const { offsetX, offsetY } = e;

            if (offsetX < this.dragEnterBoxWidth14) {
                this.classNameForTabDrag =
                    KfLayoutTargetDirectionClassName.left;
            } else if (offsetX > this.dragEnterBoxWidth34) {
                this.classNameForTabDrag =
                    KfLayoutTargetDirectionClassName.right;
            } else if (offsetY < this.dragEnterBoxHeight14) {
                this.classNameForTabDrag = KfLayoutTargetDirectionClassName.top;
            } else if (offsetY > this.dragEnterBoxHeight34) {
                this.classNameForTabDrag =
                    KfLayoutTargetDirectionClassName.bottom;
            } else {
                this.classNameForTabDrag =
                    KfLayoutTargetDirectionClassName.center;
            }

            e.preventDefault();
        },

        handleDragLeave() {
            this.classNameForTabDrag = KfLayoutTargetDirectionClassName.unset;
        },

        handleDragEnd() {
            this.clearState();
            this.markIsBoardDragging(false);
            this.$bus.next({
                tag: 'resize',
            } as ResizeEvent);
        },

        handleDrop() {
            this.afterDragMoveBoard(
                this.dragedContentData,
                this.boardId,
                this.classNameForTabDrag,
            );
            this.clearState();
        },

        hanldeEdit(targetContentId: KfLayout.BoardInfo['current']) {
            const targetBoardId = this.boardId;
            this.removeBoardByContentId(targetBoardId, targetContentId || '');
        },

        handleClickTab(e: KfLayout.BoardInfo['current']) {
            this.setBoardsMapAttrById(this.boardId, 'current', e?.toString());
        },

        clearState() {
            this.classNameForTabDrag = KfLayoutTargetDirectionClassName.unset;
            this.dragEnterBoxWidth14 = 0;
            this.dragEnterBoxWidth34 = 0;
            this.dragEnterBoxHeight14 = 0;
            this.dragEnterBoxHeight34 = 0;
            this.setDragedContentData(-1, '');
        },

        hasComponent(cname: string) {
            return this.$registedKfUIComponents.includes(cname);
        },
    },
});
</script>
<style lang="less">
.ant-tabs.ant-tabs-card.ant-tabs-small {
    > .ant-tabs-nav .ant-tabs-tab {
        padding: 0;
        position: relative;

        .kf-tab-header {
            padding: 6px 36px 6px 16px;
        }

        .ant-tabs-tab-remove {
            position: absolute;
            right: 16px;
        }
    }

    .ant-tabs-content-holder {
        position: relative;
        &::before {
            content: '';
            position: absolute;
            z-index: 10;
            width: 0;
            height: 0;
            display: none;
            transition: all 0.1s ease;
            background: @divider-color;
        }

        .ant-tabs-content.ant-tabs-content-top {
            height: 100%;
        }
    }

    &.drag-over-left {
        .ant-tabs-content-holder {
            &::before {
                width: 50%;
                height: 100%;
                left: 0;
                top: 0;
                display: block;
            }
        }
    }

    &.drag-over-right {
        .ant-tabs-content-holder {
            &::before {
                width: 50%;
                height: 100%;
                left: 50%;
                top: 0;
                display: block;
            }
        }
    }

    &.drag-over-top {
        .ant-tabs-content-holder {
            &::before {
                width: 100%;
                height: 50%;
                left: 0;
                top: 0;
                display: block;
            }
        }
    }

    &.drag-over-bottom {
        .ant-tabs-content-holder {
            &::before {
                width: 100%;
                height: 50%;
                left: 0;
                top: 50%;
                display: block;
            }
        }
    }

    &.drag-over {
        .ant-tabs-content-holder {
            &::before {
                width: 100%;
                height: 100%;
                left: 0;
                top: 0;
                display: block;
            }
        }
    }

    &.is-dragging {
        .ant-tabs-content-holder {
            * {
                pointer-events: none;
            }
        }
    }
}
</style>
