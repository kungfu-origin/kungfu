import {
  buildConfigGetterWrapByComputed,
  buildTableColumnSorter,
} from './../../../utils/index';
import { useBasketTradeStore } from './../../../store';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { BasketOrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { BASKET_CATEGORYS } from '../../../config';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = buildConfigGetterWrapByComputed(
  () =>
    [
      {
        type: 'string',
        name: t('BasketTrade.insert_time'),
        dataIndex: 'insert_time',
        width: 120,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'insert_time',
        ),
      },
      {
        type: 'string',
        name: '',
        dataIndex: 'side',
        width: 40,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'side',
        ),
      },
      {
        type: 'number',
        name: t('BasketTrade.price'),
        dataIndex: 'price_resolved',
        width: 80,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'num',
          'price_resolved',
        ),
      },
      {
        type: 'string',
        name: t('BasketTrade.status'),
        dataIndex: 'status_uname',
        width: 80,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'status_uname',
        ),
      },
      {
        type: 'string',
        name: t('BasketTrade.progress'),
        dataIndex: 'progress',
        width: 120,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'num',
          'progress',
        ),
      },
      {
        name: t('BasketTrade.actions'),
        dataIndex: 'actions',
        width: 80,
      },
    ] as KfTradingDataTableHeaderConfig[],
);

export const BasketOrderStatus: Record<
  BasketOrderStatusEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [BasketOrderStatusEnum.Unknown]: {
    name: t('tradingConfig.unknown'),
    color: 'default',
  },
  [BasketOrderStatusEnum.Pending]: {
    name: t('BasketTrade.pending'),
    color: 'default',
  },
  [BasketOrderStatusEnum.PartialFilledNotActive]: {
    name: t('BasketTrade.partial_filled_not_active'),
    color: 'default',
  },
  [BasketOrderStatusEnum.PartialFilledActive]: {
    name: t('BasketTrade.partial_filled_active'),
    color: 'green',
  },
  [BasketOrderStatusEnum.Filled]: {
    name: t('BasketTrade.filled'),
    color: 'green',
  },
};

const { getBasketOrderByOrderLocation } = useBasketTradeStore();

export const basketTradingDataGetter: DealTradingDataGetter = {
  category: BASKET_CATEGORYS.ORDER,
  commonData: {
    name: t('BasketTrade.basket_order'),
  },
  order: {
    getter: (
      watcher: KungfuApi.Watcher,
      orders: KungfuApi.DataTable<KungfuApi.Order>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      const basketOrder = getBasketOrderByOrderLocation(kfLocation);

      if (basketOrder) {
        return orders
          .nofilter('parent_id', '')
          .sort('update_time')
          .filter((order) => order.parent_id === basketOrder.order_id);
      }

      return orders.sort('update_time');
    },
  },
  trade: {
    getter: (
      watcher: KungfuApi.Watcher,
      trades: KungfuApi.DataTable<KungfuApi.Trade>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      kfLocation;
      return trades.list();
    },
  },
  position: {
    getter: (
      watcher: KungfuApi.Watcher,
      positions: KungfuApi.DataTable<KungfuApi.Position>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      kfLocation;
      return positions.list();
    },
  },
};
