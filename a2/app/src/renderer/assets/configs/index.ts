import { KfLayoutDirection } from '@root/src/typings/enums';

export const defaultBoardsMap: KfLayout.BoardsMap = {
    0: {
        paId: -1,
        direction: KfLayoutDirection.h,
        children: [1, 2],
    },
    1: {
        paId: 0,
        direction: KfLayoutDirection.v,
        children: [4, 5, 6],
        width: '60%',
    },
    2: {
        paId: 0,
        direction: KfLayoutDirection.v,
        children: [8, 9, 10],
    },
    4: {
        paId: 1,
        direction: KfLayoutDirection.h,
        contents: ['交易账户'],
        current: '交易账户',
    },
    5: {
        paId: 1,
        direction: KfLayoutDirection.h,
        contents: ['策略进程'],
        current: '策略进程',
    },
    6: {
        paId: 1,
        direction: KfLayoutDirection.v,
        contents: ['委托记录', '成交记录'],
        current: '委托记录',
    },
    8: {
        paId: 2,
        direction: KfLayoutDirection.h,
        contents: ['行情源'],
        current: '行情源',
    },
    9: {
        paId: 2,
        direction: KfLayoutDirection.h,
        contents: ['持仓'],
        current: '持仓',
    },
    10: {
        paId: 2,
        direction: KfLayoutDirection.h,
        children: [11, 12],
    },
    11: {
        paId: 10,
        direction: KfLayoutDirection.v,
        contents: ['深度行情'],
        current: '深度行情',
    },
    12: {
        paId: 10,
        direction: KfLayoutDirection.v,
        contents: ['下单面板'],
        current: '下单面板',
    },
};

export const getBoardsMap = (): KfLayout.BoardsMap | null => {
    const data = localStorage.getItem('boardsMap');
    if (!data) {
        return null;
    } else {
        return JSON.parse(data) as KfLayout.BoardsMap;
    }
};
