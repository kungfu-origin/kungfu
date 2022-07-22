import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
const { t } = VueI18n.global;

export const columns: AntTableColumns = [
  {
    title: t('posGlobalConfig.instrument_id'),
    dataIndex: 'instrument_id',
    align: 'left',
    width: 120,
    fixed: 'left',
    minWidth: 120,
  },
  {
    title: '',
    dataIndex: 'direction',
    width: 40,
    align: 'left',
    fixed: 'left',
    minWidth: 40,
  },
  {
    title: t('posGlobalConfig.yesterday_volume'),
    dataIndex: 'yesterday_volume',
    align: 'right',
    width: 80,
  },
  {
    title: t('posGlobalConfig.today_volume'),
    dataIndex: 'today_volume',
    align: 'right',
    width: 80,
  },
  {
    title: t('posGlobalConfig.sum_volume'),
    dataIndex: 'volume',
    align: 'right',
    width: 80,
  },
  {
    title: t('posGlobalConfig.avg_open_price'),
    dataIndex: 'avg_open_price',
    align: 'right',
    width: 110,
  },
  {
    title: t('posGlobalConfig.last_price'),
    dataIndex: 'last_price',
    align: 'right',
    width: 110,
  },
  {
    title: t('posGlobalConfig.unrealized_pnl'),
    dataIndex: 'unrealized_pnl',
    align: 'right',
    width: 110,
    fixed: 'right',
  },
];

export const categoryRegisterConfig: DealTradingDataGetter = {
  category: 'globalPos',
  commonData: {
    name: t('PosGlobal'),
    color: 'pink',
  },
  order: {
    getter(watcher, orders, kfLocation) {
      const { group, name } = kfLocation;
      return orders
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort('update_time');
    },
  },
  trade: {
    getter(watcher, trades, kfLocation) {
      const { group, name } = kfLocation;
      return trades
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort('trade_time');
    },
  },
  position: {
    getter(watcher, position, kfLocation) {
      const { group, name } = kfLocation;
      return position
        .nofilter('volume', BigInt(0))
        .filter('ledger_category', LedgerCategoryEnum.td)
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort('instrument_id')
        .reverse();
    },
  },
};
