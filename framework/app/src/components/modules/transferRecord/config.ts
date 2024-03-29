import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = (): AntTableColumns => [
  {
    title: t('fund_trans.update_time'),
    dataIndex: 'update_time',
    align: 'center',
    width: 120,
  },
  {
    title: t('fund_trans.status'),
    dataIndex: 'status',
    align: 'center',
    width: 80,
  },
  {
    title: t('fund_trans.source'),
    dataIndex: 'source',
    align: 'center',
    width: 80,
  },
  {
    title: t('fund_trans.target'),
    dataIndex: 'target',
    align: 'center',
    width: 80,
  },
  {
    title: t('fund_trans.trans_type'),
    dataIndex: 'trans_type',
    align: 'center',
    width: 120,
  },
  {
    title: t('fund_trans.amount'),
    dataIndex: 'amount',
    align: 'center',
    width: 80,
  },
];
