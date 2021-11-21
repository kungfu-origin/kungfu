import { defineStore } from 'pinia';
import {
    BoardsMap,
    BoardInfo,
    ContentId,
    BoardId,
} from '@/components/global/typings';

interface GlobalState {
    boardsMap: BoardsMap;
}

export const useGlobalStore = defineStore('global', {
    state: (): GlobalState => {
        return {
            boardsMap: {},
        };
    },

    actions: {
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
                this.removeBoardByBoardId(targetBoardId);
            }
        },

        removeBoardByBoardId(targetBoardId: number) {
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
                    this.removeBoardByBoardId(paId);
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
    },
});
