import { KfCategoryRegisterProps } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

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
      const currentUID = window.watcher.getLocationUID(kfLocation);

      return orders.filter('dest', currentUID).sort('trade_time');
    },
  },
  trade: {
    getter(trades, kfLocation: KungfuApi.KfExtraLocation) {
      const currentUID = window.watcher.getLocationUID(kfLocation);

      return trades.filter('dest', currentUID).sort('trade_time');
    },
  },
  position: {
    getter(position, kfLocation: KungfuApi.KfExtraLocation) {
      const currentUID = window.watcher.getLocationUID(kfLocation);

      return position
        .nofilter('volume', BigInt(0))
        .filter('dest', currentUID)
        .sort('instrument_id')
        .reverse();
    },
  },
};
