export const getColumns = (smallMode = false): AntTableColumns => [
    {
        title: '标的',
        dataIndex: 'instrumentId',
        align: 'left',
        sorter: true,
        width: 80,
    },
    {
        title: '最新价',
        dataIndex: 'lastPrice',
        align: 'center',
        sorter: true,
        width: 60,
    },
    ...(smallMode
        ? []
        : [
              {
                  title: '成交量',
                  dataIndex: 'volume',
                  align: 'center',
                  sorter: true,
                  width: 60,
              },
          ]),
    {
        title: '操作',
        dataIndex: 'actions',
        align: 'right',
        width: 30,
        fixed: 'right',
    },
];
