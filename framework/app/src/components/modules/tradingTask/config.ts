import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;
import { KfCategoryRegisterProps } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

export const columns: AntTableColumns = [
  {
    title: t('tradingTaskConfig.task_id'),
    dataIndex: 'name',
    align: 'left',
    width: 120,
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
    width: 280,
  },
  {
    title: t('tradingTaskConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];

export const categoryRegisterConfig: KfCategoryRegisterProps = {
  name: 'tradingTask',
  commonData: {
    name: t('TradingTask'),
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
        .filter('holder_uid', currentUID)
        .sort('instrument_id')
        .reverse();
    },
  },
};
