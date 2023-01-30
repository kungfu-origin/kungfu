import { BasketVolumeTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { BasketVolumeType } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { getTradingDataSortKey } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  buildConfigGetterWrapByComputed,
  buildTableColumnSorter,
} from '../../../utils';
import { BASKET_CATEGORYS } from '../../../config';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = buildConfigGetterWrapByComputed(
  (getBasketMarkedValue: (basket: KungfuApi.BasketResolved) => number) =>
    [
      {
        title: t('BasketTrade.basket_name'),
        dataIndex: 'name',
        width: 60,
        sorter: {
          compare: buildTableColumnSorter<KungfuApi.BasketResolved>(
            'str',
            'name',
          ),
        },
      },
      {
        title: t('BasketTrade.volume_type'),
        dataIndex: 'volume_type_resolved',
        width: 40,
        sorter: {
          compare: buildTableColumnSorter<KungfuApi.BasketResolved>(
            'str',
            'volume_type',
          ),
        },
      },
      {
        title: t('tdConfig.marked_value'),
        dataIndex: 'marked_value',
        width: 60,
        sorter: {
          compare: (
            a: KungfuApi.BasketResolved,
            b: KungfuApi.BasketResolved,
          ) => {
            return getBasketMarkedValue(a) - getBasketMarkedValue(b);
          },
        },
      },
      {
        title: t('BasketTrade.total_amount'),
        dataIndex: 'total_amount',
        width: 60,
        sorter: {
          compare: buildTableColumnSorter<KungfuApi.BasketResolved>(
            'num',
            'total_amount',
          ),
        },
      },
      {
        title: '',
        dataIndex: 'actions',
        align: 'right',
        width: 60,
        fixed: 'right',
      },
    ] as AntTableColumns,
);

export const getBasketFormSettings = (volumeType: BasketVolumeTypeEnum) =>
  [
    {
      key: 'name',
      name: t('BasketTrade.basket_name'),
      type: 'str',
      primary: true,
      required: true,
      tip: t('BasketTrade.need_only_basket_name_tip'),
    },
    {
      key: 'volume_type',
      name: t('BasketTrade.volume_type'),
      type: 'radio',
      options: [
        {
          label: BasketVolumeType[BasketVolumeTypeEnum.Quantity].name,
          value: `${BasketVolumeTypeEnum.Quantity}`,
          type: 'tag',
          color: BasketVolumeType[BasketVolumeTypeEnum.Quantity].color,
        },
        {
          label: BasketVolumeType[BasketVolumeTypeEnum.Proportion].name,
          value: `${BasketVolumeTypeEnum.Proportion}`,
          type: 'tag',
          color: BasketVolumeType[BasketVolumeTypeEnum.Proportion].color,
        },
      ],
      default: `${BasketVolumeTypeEnum.Quantity}`,
      primary: true,
      required: true,
    },
    ...(volumeType === BasketVolumeTypeEnum.Proportion
      ? [
          {
            key: 'total_amount',
            name: t('BasketTrade.total_asset'),
            type: 'int',
            required: true,
            min: 1,
            default: 1,
          },
        ]
      : []),
  ] as KungfuApi.KfConfigItem[];

const orderSortKey = getTradingDataSortKey('Order');
const tradeSortKey = getTradingDataSortKey('Trade');
const positionSortKey = getTradingDataSortKey('Position');
export const basketTradingDataGetter: DealTradingDataGetter = {
  category: BASKET_CATEGORYS.SETTING,
  commonData: {
    name: t('BasketTrade.basket'),
    color: 'blue',
  },
  order: {
    getter: (
      watcher: KungfuApi.Watcher,
      orders: KungfuApi.DataTable<KungfuApi.Order>,
      kfLocation: KungfuApi.KfLocation,
    ) => {
      // const basketOrders = getBasketOrdersByBasketLocation(kfLocation);
      // const basketOrderIds = Object.keys(basketOrders).join('_');

      // if (basketOrderIds.length) {
      //   return orders
      //     .nofilter('parent_id', '')
      //     .sort('update_time')
      //     .filter((order) => {
      //       return new RegExp(`.*${order.parent_id}.*`, 'ig').test(
      //         basketOrderIds,
      //       );
      //     });
      // }
      kfLocation;

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
