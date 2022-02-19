export const columns: AntTableColumns = [
  {
    title: '任务ID',
    dataIndex: 'name',
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
    title: '参数',
    dataIndex: 'args',
    align: 'left',
    width: 280,
  },
  {
    title: '操作',
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];
