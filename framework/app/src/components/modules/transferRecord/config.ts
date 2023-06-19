import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = (): AntTableColumns => [
  {
    title: t('account_move.trading_day'),
    dataIndex: 'trading_day',
    align: 'center',
    width: 120,
  },
  {
    title: t('account_move.source'),
    dataIndex: 'source',
    align: 'center',
    width: 80,
  },
  {
    title: t('account_move.target'),
    dataIndex: 'target',
    align: 'center',
    width: 80,
  },
  {
    title: t('account_move.amount'),
    dataIndex: 'amount',
    align: 'center',
    width: 80,
  },
];
