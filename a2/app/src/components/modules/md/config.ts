export const columns: AntTableColumns = [
    {
        title: '柜台',
        dataIndex: 'name',
        align: 'left',
        sorter: true,
        width: 60,
    },
    {
        title: '状态',
        dataIndex: 'stateStatus',
        align: 'left',
        sorter: true,
        width: 80,
    },
    {
        title: '进程',
        dataIndex: 'processStatus',
        align: 'center',
        sorter: true,
        width: 60,
    },
    {
        title: '操作',
        dataIndex: 'actions',
        align: 'right',
        width: 140,
        fixed: 'right',
    },
];
