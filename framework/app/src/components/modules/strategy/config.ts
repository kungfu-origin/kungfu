import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

export const getColumns = (
  sorter: (
    dataIndex: string,
  ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
): AntTableColumns => [
  {
    title: t('strategyConfig.strategy_id'),
    dataIndex: 'name',
    align: 'left',
    width: 90,
    fixed: 'left',
  },
  {
    title: t('strategyConfig.strategy_file'),
    dataIndex: 'strategyFile',
    align: 'left',
    width: 120,
    fixed: 'left',
  },
  {
    title: t('strategyConfig.process_status'),
    dataIndex: 'processStatus',
    align: 'center',
    width: 60,
    fixed: 'left',
  },
  {
    title: t('strategyConfig.unrealized_pnl'),
    dataIndex: 'unrealizedPnl',
    align: 'right',
    sorter: {
      compare: sorter('unrealized_pnl'),
    },
    width: 110,
  },
  {
    title: t('strategyConfig.marked_value'),
    dataIndex: 'marketValue',
    align: 'right',
    sorter: {
      compare: sorter('market_value'),
    },
    width: 110,
  },
  {
    title: t('strategyConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];
