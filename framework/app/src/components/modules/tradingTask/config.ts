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

export const categoryRegisterConfig: KfCategoryRegisterProps = {
  name: 'tradingTask',
  commonData: {
    name: '交易任务',
    color: 'purple',
  },
  order: {
    getter(orders, kfLocation: KungfuApi.KfExtraLocation) {
      const realLocation: KungfuApi.KfExtraLocation = {
        ...kfLocation,
        category: 'strategy',
      };
      const currentUID = window.watcher.getLocationUID(realLocation);
      return orders.filter('dest', currentUID).sort('trade_time');
    },
  },
  trade: {
    getter(trades, kfLocation: KungfuApi.KfExtraLocation) {
      const realLocation: KungfuApi.KfExtraLocation = {
        ...kfLocation,
        category: 'strategy',
      };
      const currentUID = window.watcher.getLocationUID(realLocation);
      return trades.filter('dest', currentUID).sort('trade_time');
    },
  },
  position: {
    getter(position, kfLocation: KungfuApi.KfExtraLocation) {
      const realLocation: KungfuApi.KfExtraLocation = {
        ...kfLocation,
        category: 'strategy',
      };
      const currentUID = window.watcher.getLocationUID(realLocation);
      return position
        .nofilter('volume', BigInt(0))
        .filter('dest', currentUID)
        .sort('instrument_id')
        .reverse();
    },
  },
};
