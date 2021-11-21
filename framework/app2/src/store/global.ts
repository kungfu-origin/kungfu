import { defineStore } from 'pinia';
import { BoardsMap, BoardInfo } from '@/components/global/typings';

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
            this.$state.boardsMap = boardsMap;
        },

        setBoardsMapAttrById(
            id: number,
            attrKey: keyof BoardInfo,
            value: BoardInfo[keyof BoardInfo],
        ) {
            (<typeof value>this.$state.boardsMap[id][attrKey]) = value;
        },
    },
});
