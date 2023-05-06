import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
type ContentId = string;
type BoardId = number;

interface BoardInfo {
  paId: number;
  children?: number[];
  direction: 'h' | 'v';
  contents?: ContentId[];
  current?: ContentId;
  width?: number | string;
  height?: number | string;
}

interface BoardsMap {
  [prop: BoardId]: BoardInfo;
}

type dealBoardsMapCB = (boardsMap: BoardsMap) => BoardsMap;

export class DealBoardsMapHook {
  callbacks: Array<dealBoardsMapCB>;

  constructor() {
    this.callbacks = [];
  }

  register(callback: dealBoardsMapCB) {
    this.callbacks.push(callback);
    kfLogger.info('DealBoardsMapHook has registered.');
  }

  trigger(defaultBoardsMap: BoardsMap) {
    return this.callbacks.reduce((boardsMap, callback) => {
      return callback(boardsMap);
    }, defaultBoardsMap as BoardsMap);
  }
}
