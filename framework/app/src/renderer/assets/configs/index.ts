export const defaultBoardsMap: KfLayout.BoardsMap = {
    '0': { paId: -1, direction: 'h', children: [1, 2] },
    '1': { paId: 0, direction: 'v', children: [4, 5], width: '60.326%' },
    '2': { paId: 0, direction: 'v', children: [8, 9, 10], width: '39.674%' },
    '4': {
        paId: 1,
        direction: 'h',
        contents: ['交易账户'],
        current: '交易账户',
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
        width: '43.279%',
    },
    '12': {
        paId: 10,
        direction: 'v',
        contents: ['下单面板'],
        current: '下单面板',
        width: '56.721%',
    },
    '13': {
        paId: 5,
        direction: 'v',
        contents: ['行情订阅'],
        current: '行情订阅',
        width: '27.945%',
    },
    '14': { paId: 5, direction: 'v', width: '72.055%', children: [16, 15] },
    '15': {
        paId: 14,
        direction: 'h',
        contents: ['委托记录', '成交记录'],
        current: '委托记录',
        width: 0,
        height: 0,
    },
    '16': {
        paId: 14,
        direction: 'h',
        contents: ['策略进程'],
        current: '策略进程',
    },
};

export const getBoardsMap = (): KfLayout.BoardsMap | null => {
    const data = localStorage.getItem('boardsMap');
    if (!data) {
        return null;
    } else {
        const storedBoardsMap = JSON.parse(data) as KfLayout.BoardsMap;
        if (!Object.keys(storedBoardsMap).length) {
            return null;
        }

        return storedBoardsMap;
    }
};
