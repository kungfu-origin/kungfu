import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = (): AntTableColumns => [
  {
    title: t('fund_trans.trading_day'),
    dataIndex: 'trading_day',
    align: 'center',
    width: 120,
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
    title: t('fund_trans.amount'),
    dataIndex: 'amount',
    align: 'center',
    width: 80,
  },
];
