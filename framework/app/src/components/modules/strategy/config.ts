export const getColumns = (
    sorter: (
        dataIndex: string,
    ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
): AntTableColumns => [
    {
        title: '策略ID',
        dataIndex: 'name',
        align: 'left',
        width: 90,
        fixed: 'left',
    },
    {
        title: '策略文件',
        dataIndex: 'strategyFile',
        align: 'left',
        width: 120,
        fixed: 'left',
    },
    {
        title: '进程',
        dataIndex: 'processStatus',
        align: 'center',
        width: 60,
        fixed: 'left',
    },
    {
        title: '浮动盈亏',
        dataIndex: 'unrealizedPnl',
        align: 'right',
        sorter: {
            compare: sorter('unrealized_pnl'),
        },
        width: 110,
    },
    {
        title: '市值',
        dataIndex: 'marketValue',
        align: 'right',
        sorter: {
            compare: sorter('market_value'),
        },
        width: 110,
    },
    {
        title: '操作',
        dataIndex: 'actions',
        align: 'right',
        width: 140,
        fixed: 'right',
    },
];
