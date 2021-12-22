import { KfLayoutDirection } from '@root/src/typings/enums';

export const defaultBoardsMap: KfLayout.BoardsMap = {
    '0': { paId: -1, direction: 'h', children: [1, 2] },
    '1': { paId: 0, direction: 'v', children: [4, 5, 6], width: '55.054%' },
    '2': { paId: 0, direction: 'v', children: [8, 9, 10], width: '44.946%' },
    '4': {
        paId: 1,
        direction: 'h',
        contents: ['交易账户'],
        current: '交易账户',
        height: '34.044%',
    },
    '5': {
        paId: 1,
        direction: 'h',
        contents: ['策略进程'],
        current: '策略进程',
        height: '18.601%',
    },
    '6': {
        paId: 1,
        direction: 'v',
        contents: ['委托记录', '成交记录'],
        current: '委托记录',
        height: '47.355%',
    },
    '8': {
        paId: 2,
        direction: 'h',
        contents: ['行情源'],
        current: '行情源',
        height: '17.577%',
    },
    '9': {
        paId: 2,
        direction: 'h',
        contents: ['持仓'],
        current: '持仓',
        height: '37.031%',
    },
    '10': { paId: 2, direction: 'h', children: [11, 12], height: '45.392%' },
    '11': {
        paId: 10,
        direction: 'v',
        contents: ['深度行情'],
        current: '深度行情',
        width: '41.576%',
    },
    '12': {
        paId: 10,
        direction: 'v',
        contents: ['下单面板'],
        current: '下单面板',
        width: '58.424%',
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
