import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const columns: AntTableColumns = [
  {
    title: t('tradingTaskConfig.task_id'),
    dataIndex: 'name',
    align: 'left',
    width: 120,
    fixed: 'left',
  },
  {
    title: t('tdConfig.state_status'),
    dataIndex: 'stateStatus',
    align: 'left',
    width: 80,
    fixed: 'left',
  },
  {
    title: t('tradingTaskConfig.process_status'),
    dataIndex: 'processStatus',
    align: 'center',
    width: 60,
    fixed: 'left',
  },
  {
    title: t('tradingTaskConfig.args'),
    dataIndex: 'args',
    align: 'left',
    width: 250,
  },
  {
    title: t('tradingTaskConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 110,
    fixed: 'right',
  },
];
