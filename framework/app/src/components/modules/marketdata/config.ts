export const getColumns = (smallMode = false): AntTableColumns => [
  {
    title: '标的',
    dataIndex: 'instrumentId',
    align: 'left',
    width: 80,
  },
  ...(smallMode
    ? []
    : [
        {
          title: '开盘价',
          dataIndex: 'open_price',
          align: 'right',
          width: 80,
        },
        {
          title: '最低价',
          dataIndex: 'low_price',
          align: 'right',
          width: 80,
        },
        {
          title: '最高价',
          dataIndex: 'high_price',
          align: 'right',
          width: 80,
        },
      ]),
  {
    title: '最新价',
    dataIndex: 'lastPrice',
    align: 'right',
    width: 80,
  },
  ...(smallMode
    ? []
    : [
        {
          title: '成交量',
          dataIndex: 'volume',
          align: 'right',
          width: 60,
        },
      ]),
  {
    title: '操作',
    dataIndex: 'actions',
    align: 'right',
    width: 40,
    fixed: 'right',
  },
];
