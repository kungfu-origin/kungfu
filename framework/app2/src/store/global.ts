import { defineStore } from 'pinia';
import { BoardsMap } from '@/components/global/typings';

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
        setBoardsMap(boardsMap: BoardsMap) {
            this.$state.boardsMap = boardsMap;
        },
    },
});
