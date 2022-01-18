export const getColumns = (
    sorter: (
        dataIndex: string,
    ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
): AntTableColumns => [
    {
        title: '账户(组)',
        dataIndex: 'name',
        align: 'left',
        width: 180,
        fixed: 'left',
    },
    {
        title: '备注',
        dataIndex: 'accountName',
        align: 'left',
        width: 80,
        fixed: 'left',
    },
    {
        title: '状态',
        dataIndex: 'stateStatus',
        align: 'left',
        width: 80,
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
        title: '保证金',
        dataIndex: 'margin',
        align: 'right',
        sorter: {
            compare: sorter('margin'),
        },
        width: 110,
    },
    {
        title: '可用资金',
        dataIndex: 'avail',
        align: 'right',
        sorter: {
            compare: sorter('avail'),
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
