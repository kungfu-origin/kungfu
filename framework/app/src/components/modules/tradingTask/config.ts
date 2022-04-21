import { KfCategoryRegisterProps } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';
import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';

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

export const tradeRegisterConfig: KfCategoryRegisterProps = {
  name: 'globalTrade',
  commonData: {
    name: '交易任务',
    color: 'pink',
  },
  order: {
    getter(orders, kfLocation: KungfuApi.KfExtraLocation) {
      const { group, name } = kfLocation;
      return orders
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort('update_time');
    },
  },
  trade: {
    getter(trades, kfLocation: KungfuApi.KfExtraLocation) {
      const { group, name } = kfLocation;
      return trades
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort('trade_time');
    },
  },
  position: {
    getter(position, kfLocation: KungfuApi.KfExtraLocation) {
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
