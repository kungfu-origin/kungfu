import {
  buildConfigGetterWrapByComputed,
  buildTableColumnSorter,
} from './../../../utils/index';
import { useBasketTradeStore } from './../../../store';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { BasketOrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { getTradingDataSortKey } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
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
        width: 100,
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
        type: 'string',
        name: t('BasketTrade.price'),
        dataIndex: 'price_level',
        width: 60,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'price_level',
        ),
      },
      {
        type: 'number',
        name: t('tradingConfig.price_offset'),
        dataIndex: 'price_offset',
        width: 60,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'num',
          'price_offset',
        ),
      },
      {
        type: 'string',
        name: t('BasketTrade.status'),
        dataIndex: 'status_uname',
        width: 70,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'status_uname',
        ),
      },
      {
        type: 'string',
        name: `${t('orderConfig.clinch')}/${t('orderConfig.all')}`,
        dataIndex: 'volume_left',
        width: 110,
        sorter: (
          a: KungfuApi.BasketOrderResolved,
          b: KungfuApi.BasketOrderResolved,
        ) => {
          return Number(
            Number(a.volume - a.volume_left) - Number(b.volume - b.volume_left),
          );
        },
      },
      {
        type: 'string',
        name: t('BasketTrade.progress'),
        dataIndex: 'progress',
        width: 200,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'num',
          'progress',
        ),
      },
      {
        type: 'source',
        name: t('orderConfig.dest_uname'),
        dataIndex: 'dest_uname',
        width: 100,
        sorter: buildTableColumnSorter<KungfuApi.BasketOrderResolved>(
          'str',
          'dest_uname',
        ),
      },
      {
        name: '',
        dataIndex: 'actions',
        width: 50,
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
    name: t('tradingConfig.pending'),
    color: 'default',
  },
  [BasketOrderStatusEnum.PartialFilledNotActive]: {
    name: t('tradingConfig.partial_filled_not_active'),
    color: 'green',
  },
  [BasketOrderStatusEnum.PartialFilledActive]: {
    name: t('tradingConfig.partial_filled_active'),
    color: 'default',
  },
  [BasketOrderStatusEnum.Filled]: {
    name: t('tradingConfig.filled'),
    color: 'green',
  },
};

const { getBasketOrderByOrderLocation } = useBasketTradeStore();

const orderSortKey = getTradingDataSortKey('Order');
const tradeSortKey = getTradingDataSortKey('Trade');
const positionSortKey = getTradingDataSortKey('Position');
export const basketOrderTradingDataGetter: DealTradingDataGetter = {
  category: BASKET_CATEGORYS.ORDER,
  commonData: {
    name: t('BasketTrade.basket_order'),
    color: 'green',
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
          .filter('parent_id', basketOrder.order_id)
          .sort(orderSortKey);
      }

      return orders.sort(orderSortKey);
    },
  },
  trade: {
    getter: (
      watcher: KungfuApi.Watcher,
      trades: KungfuApi.DataTable<KungfuApi.Trade>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      kfLocation;
      return trades.sort(tradeSortKey);
    },
  },
  position: {
    getter: (
      watcher: KungfuApi.Watcher,
      positions: KungfuApi.DataTable<KungfuApi.Position>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      kfLocation;
      return positions.sort(positionSortKey);
    },
  },
};
