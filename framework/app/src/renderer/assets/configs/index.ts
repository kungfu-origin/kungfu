import { useBoardFilter } from '../methods/uiUtils';

const { getBoard } = useBoardFilter();

const getBoardSizeNum = (size: string | number | undefined) => {
  if (typeof size === 'string') {
    if (size.endsWith('%')) {
      return Number(size.slice(0, size.length - 1));
    }
  }

  return 0;
};

const dealBoardMap = (boardsMap: KfLayout.BoardsMap) => {
  const dealBoard = (numKey: number) => {
    const currentBoard = boardsMap[numKey];

    if (currentBoard) {
      if (currentBoard.children) {
        const childrenResolved = currentBoard.children
          .map((childKey) => dealBoard(childKey))
          .filter((item) => item !== -1);
        if (!childrenResolved.length) {
          return -1;
        } else {
          let residue = 0;
          const curSizeKey =
            currentBoard.direction === 'v' ? 'height' : 'width';

          currentBoard.children = currentBoard.children.filter((childId) => {
            if (childrenResolved.includes(childId)) {
              return true;
            } else {
              const curSize = boardsMap[childId][curSizeKey];
              residue += getBoardSizeNum(curSize);
              delete boardsMap[childId];
              return false;
            }
          });

          const everySize = residue / currentBoard.children.length;

          currentBoard.children.forEach((childKey) => {
            const curChildBoardSize = getBoardSizeNum(
              boardsMap[childKey][curSizeKey],
            );
            boardsMap[childKey][curSizeKey] = `${
              curChildBoardSize + everySize
            }%`;
          });

          return numKey;
        }
      }

      if (currentBoard.contents?.length) {
        const contentsResolved = currentBoard.contents
          .map((contentId) => getBoard(contentId, contentId, ''))
          .filter((item) => !!item);
        if (!contentsResolved.length) {
          return -1;
        } else {
          currentBoard.contents = contentsResolved;
          currentBoard.current = contentsResolved[0];
          return numKey;
        }
      }
    }

    return -1;
  };

  dealBoard(0);

  return boardsMap;
};

const baseBoardsMap: KfLayout.BoardsMap = {
  '0': { paId: -1, direction: 'h', children: [1, 2] },
  '1': { paId: 0, direction: 'v', children: [4, 5], width: '64.620%' },
  '2': { paId: 0, direction: 'v', children: [8, 9, 10], width: '35.380%' },
  '4': {
    paId: 1,
    direction: 'h',
    contents: ['Td'],
    current: 'Td',
    height: '23.294%',
    width: 0,
  },
  '5': {
    paId: 1,
    direction: 'h',
    children: [14, 13],
    width: 0,
    height: '76.706%',
  },
  '8': {
    paId: 2,
    direction: 'h',
    contents: ['Md'],
    current: 'Md',
    height: '17.577%',
  },
  '9': {
    paId: 2,
    direction: 'h',
    contents: ['Pos'],
    current: 'Pos',
    height: '28.157%',
  },
  '10': { paId: 2, direction: 'h', children: [11, 12], height: '54.266%' },
  '11': {
    paId: 10,
    direction: 'v',
    contents: ['OrderBook'],
    current: 'OrderBook',
    width: '41.424%',
  },
  '12': {
    paId: 10,
    direction: 'v',
    contents: ['MakeOrder'],
    current: 'MakeOrder',
    width: '58.576%',
  },
  '13': {
    paId: 5,
    direction: 'v',
    contents: ['MarketData'],
    current: 'MarketData',
    width: '23.045%',
  },
  '14': { paId: 5, direction: 'v', width: '76.955%', children: [16, 15] },
  '15': {
    paId: 14,
    direction: 'h',
    contents: ['Order', 'Trade'],
    current: 'Order',
    height: '50%',
  },
  '16': {
    paId: 14,
    direction: 'h',
    contents: ['Strategy', 'TradingTask', 'PosGlobal'],
    current: 'Strategy',
    height: '50%',
  },
};

export const defaultBoardsMap = dealBoardMap(baseBoardsMap);

export const getIndexBoardsMap = (): KfLayout.BoardsMap | null => {
  const data = localStorage.getItem('indexBoardsMap');
  if (!data) {
    return null;
  }
  const storedBoardsMap = JSON.parse(data) as KfLayout.BoardsMap;
  if (!Object.keys(storedBoardsMap).length) {
    return null;
  }

  return storedBoardsMap;
};
