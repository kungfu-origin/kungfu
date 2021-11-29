import { defineStore } from 'pinia';
import { toRaw } from 'vue';
import {
    BoardsMap,
    BoardInfo,
    ContentId,
    ContentData,
    BoardId,
    Direction,
    TargetDirectionClassName,
} from '@renderer/types/index';

interface GlobalState {
    boardsMap: BoardsMap;
    dragedContentData: ContentData | null;
    isBoardDragging: boolean;
}

export const useGlobalStore = defineStore('global', {
    state: (): GlobalState => {
        return {
            boardsMap: {},
            dragedContentData: null,
            isBoardDragging: false,
        };
    },

    actions: {
        markIsBoardDragging(status: boolean) {
            this.isBoardDragging = status;
        },

        initBoardsMap(boardsMap: BoardsMap) {
            this.boardsMap = boardsMap;
        },

        setBoardsMapAttrById(
            id: number,
            attrKey: keyof BoardInfo,
            value: BoardInfo[keyof BoardInfo],
        ) {
            (<typeof value>this.boardsMap[id][attrKey]) = value;
        },

        removeBoardByContentId(targetBoardId: number, targetContentId: string) {
            const targetBoard: BoardInfo = this.boardsMap[targetBoardId];
            const contents = targetBoard?.contents;
            const targetIndex = contents?.indexOf(targetContentId);

            if (targetIndex === undefined) return;
            if (targetIndex === -1) return;

            const removedItem: ContentId =
                (contents?.splice(targetIndex, 1) || [])[0] || '';

            if (removedItem === targetBoard.current && contents?.length) {
                targetBoard.current = (targetBoard.contents || [])[0];
            }

            if (!contents?.length && targetBoard.paId != -1) {
                this.removeBoardByBoardId_(targetBoardId);
            }
        },

        removeBoardByBoardId_(targetBoardId: number) {
            const targetBoard = this.boardsMap[targetBoardId];
            if (targetBoard && targetBoard.paId !== -1) {
                const paId = targetBoard.paId;
                const paBoard = this.boardsMap[paId];
                const children = paBoard?.children;
                const childIndex = paBoard.children?.indexOf(targetBoardId);

                if (childIndex === undefined) return;
                if (childIndex === -1) return;

                children?.splice(childIndex, 1);

                if (!children?.length) {
                    this.removeBoardByBoardId_(paId);
                } else {
                    children.forEach((childId: BoardId) => {
                        this.boardsMap[childId].width = 0;
                        this.boardsMap[childId].height = 0;
                    });
                }

                delete this.boardsMap[targetBoardId];
            }
            return;
        },

        setDragedContentData(boardId: BoardId, contentId: ContentId) {
            if (boardId === -1 && !contentId) {
                this.dragedContentData = null;
            } else {
                this.dragedContentData = {
                    contentId,
                    boardId,
                };
            }
        },

        afterDragMoveBoard(
            dragedContentData: ContentData | null,
            destBoardId: BoardId,
            directionClassName: TargetDirectionClassName,
        ) {
            const { boardId, contentId } = dragedContentData || {};
            const destBoard = this.boardsMap[destBoardId];

            if (!contentId || boardId === undefined) return;
            if (
                boardId === destBoardId &&
                destBoard.contents &&
                destBoard.contents.length === 1
            )
                return;

            this.removeBoardByContentId(boardId, contentId);

            if (directionClassName === TargetDirectionClassName.center) {
                destBoard.contents?.push(contentId);
                destBoard.current = contentId;
            } else if (directionClassName != TargetDirectionClassName.unset) {
                this.dragMakeNewBoard_(
                    contentId,
                    destBoardId,
                    directionClassName,
                );
            }
        },

        dragMakeNewBoard_(
            contentId: ContentId,
            destBoardId: number,
            directionClassName: TargetDirectionClassName,
        ) {
            const destBoard = this.boardsMap[destBoardId];
            const destPaId: number = destBoard.paId;
            const destDirection: Direction = destBoard.direction;
            const newBoardId: BoardId = this.buildNewBoardId_();
            const newBoardDirection: Direction =
                directionClassName === TargetDirectionClassName.top ||
                directionClassName === TargetDirectionClassName.bottom
                    ? Direction.h
                    : directionClassName === TargetDirectionClassName.left ||
                      directionClassName === TargetDirectionClassName.right
                    ? Direction.v
                    : Direction.unset;
            const newBoardInfo: BoardInfo = {
                paId: destPaId,
                direction: newBoardDirection,
                contents: [contentId],
                current: contentId,
            };

            if (destDirection === newBoardDirection) {
                const siblings = this.boardsMap[destPaId].children;
                const destIndex = siblings?.indexOf(destBoardId);
                if (destIndex === -1 || destIndex === undefined) {
                    throw new Error(
                        "Insert dest board is not in pa board's childen",
                    );
                }

                if (
                    directionClassName === TargetDirectionClassName.top ||
                    directionClassName === TargetDirectionClassName.left
                ) {
                    siblings?.splice(destIndex, 0, newBoardId);
                } else {
                    siblings?.splice(destIndex + 1, 0, newBoardId);
                }
            } else {
                newBoardInfo.paId = destBoardId;
                const destBoardCopy: BoardInfo = {
                    ...toRaw(destBoard),
                    direction: newBoardDirection,
                    paId: destBoardId,
                };

                const newDestBoardId = newBoardId + 1;
                if (
                    directionClassName === TargetDirectionClassName.top ||
                    directionClassName === TargetDirectionClassName.left
                ) {
                    destBoard.children = [newBoardId, newDestBoardId];
                } else {
                    destBoard.children = [newDestBoardId, newBoardId];
                }
                delete destBoard.contents;
                delete destBoard.current;
                this.boardsMap[newDestBoardId] = destBoardCopy;
            }

            this.boardsMap[newBoardId] = newBoardInfo;
        },

        buildNewBoardId_(): BoardId {
            const boardIds = Object.keys(this.boardsMap)
                .map((key: string) => +key)
                .sort((key1: number, key2: number) => key2 - key1);
            return boardIds[0] + 1;
        },
    },
});
