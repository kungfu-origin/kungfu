import { Ref } from 'vue';
import { BasketVolumeTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  buildConfigGetterWrapByComputed,
  buildTableColumnSorter,
} from '../../../utils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = buildConfigGetterWrapByComputed(
  (
    basket: Ref<KungfuApi.BasketResolved | null>,
    basketInstrumentLastPriceSorter: (
      a: KungfuApi.BasketInstrumentResolved,
      b: KungfuApi.BasketInstrumentResolved,
    ) => number,
  ) =>
    [
      {
        type: 'string',
        name: t('BasketTrade.instrument_id'),
        dataIndex: 'basketInstrumentName',
        flex: 1.5,
        sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
          'str',
          'basketInstrumentName',
        ),
      },
      {
        type: 'string',
        name: t('BasketTrade.exchange_id'),
        dataIndex: 'exchange_id',
        flex: 1,
        sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
          'str',
          'exchange_id',
        ),
      },
      {
        type: 'number',
        name: t('BasketTrade.last_price'),
        dataIndex: 'last_price',
        flex: 1,
        sorter: basketInstrumentLastPriceSorter,
      },
      basket.value?.volume_type === BasketVolumeTypeEnum.Quantity
        ? {
            type: 'number',
            name: t('BasketTrade.volume'),
            dataIndex: 'volume',
            flex: 1.2,
            sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
              'num',
              'volume',
            ),
          }
        : {
            type: 'number',
            name: t('BasketTrade.rate'),
            dataIndex: 'rate',
            flex: 1.2,
            sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
              'num',
              'rate',
            ),
          },
      {
        name: t('tdConfig.actions'),
        dataIndex: 'actions',
        width: 80,
      },
    ] as KfTradingDataTableHeaderConfig[],
);

export const getSetBasketInstrumentFormSettings =
  buildConfigGetterWrapByComputed(
    (basketVolumeType: BasketVolumeTypeEnum) =>
      [
        {
          key: 'basketInstruments',
          name: t('BasketTrade.basket_instrument'),
          type: 'csvTable',
          headers: ['instrument_id', 'exchange_id', 'volume', 'rate'],
          default: [],
          columns: [
            {
              key: 'instrument',
              name: t('tradingConfig.instrument'),
              type: 'instrument',
            },
            basketVolumeType === BasketVolumeTypeEnum.Quantity
              ? {
                  key: 'volume',
                  name: t('BasketTrade.volume'),
                  type: 'int',
                  required: true,
                  min: 1,
                  default: 1,
                }
              : {
                  key: 'rate',
                  name: t('BasketTrade.rate'),
                  type: 'percent',
                  required: true,
                  min: 0.01,
                  default: 1,
                },
          ],
        },
      ] as KungfuApi.KfConfigItem[],
  );
