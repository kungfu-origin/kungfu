export const columns: AntTableColumns = [
  {
    title: '任务ID',
    dataIndex: 'processId',
    align: 'left',
    width: 160,
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
    title: '类型',
    dataIndex: 'type',
    align: 'left',
    width: 80,
  },
  {
    title: '操作',
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];
