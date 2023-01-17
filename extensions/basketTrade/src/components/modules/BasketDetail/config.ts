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
    basketInstrumentSorters: Record<
      string,
      (
        a: KungfuApi.BasketInstrumentResolved,
        b: KungfuApi.BasketInstrumentResolved,
      ) => number
    >,
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

      basket.value?.volume_type === BasketVolumeTypeEnum.Quantity
        ? {
            type: 'number',
            name: t('BasketTrade.volume'),
            dataIndex: 'volume',
            flex: 1,
            sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
              'num',
              'volume',
            ),
          }
        : {
            type: 'number',
            name: t('BasketTrade.rate'),
            dataIndex: 'rate',
            flex: 1,
            sorter: buildTableColumnSorter<KungfuApi.BasketInstrumentResolved>(
              'num',
              'rate',
            ),
          },
      {
        type: 'number',
        name: t('posGlobalConfig.sum_volume'),
        dataIndex: 'position_volume',
        width: 80,
        sorter: basketInstrumentSorters['volume'],
      },
      {
        type: 'number',
        name: t('posGlobalConfig.avg_open_price'),
        dataIndex: 'avg_open_price',
        width: 110,
        sorter: basketInstrumentSorters['avg_open_price'],
      },
      {
        type: 'number',
        name: t('BasketTrade.last_price'),
        dataIndex: 'last_price',
        flex: 1,
        sorter: basketInstrumentSorters['last_price'],
      },
      {
        type: 'number',
        name: t('posGlobalConfig.unrealized_pnl'),
        dataIndex: 'unrealized_pnl',
        width: 110,
        sorter: basketInstrumentSorters['unrealized_pnl'],
      },
      {
        name: t('tdConfig.actions'),
        dataIndex: 'actions',
        width: 100,
      },
    ] as KfTradingDataTableHeaderConfig[],
);

const getExportData = (basketVolumeType: BasketVolumeTypeEnum) => {
  const data = [
    { instrument_id: '600000', exchange_id: 'SSE', volume: 1000, rate: 0.2 },
    { instrument_id: '600002', exchange_id: 'SSE', volume: 1000, rate: 0.2 },
    { instrument_id: '600004', exchange_id: 'SSE', volume: 1000, rate: 0.2 },
    { instrument_id: '600006', exchange_id: 'SSE', volume: 1000, rate: 0.2 },
    { instrument_id: '600009', exchange_id: 'SSE', volume: 1000, rate: 0.2 },
  ];
  return data.map((item) => ({
    ...item,
    ...(basketVolumeType === BasketVolumeTypeEnum.Quantity
      ? { rate: 0 }
      : { volume: 0 }),
  }));
};

export const getSetBasketInstrumentFormSettings =
  buildConfigGetterWrapByComputed(
    (basketVolumeType: BasketVolumeTypeEnum) =>
      [
        {
          key: 'basketInstruments',
          name: t('BasketTrade.basket_instrument'),
          type: 'csvTable',
          template: [
            {
              name: '篮子标的csv模板(按比例).csv',
              data: getExportData(BasketVolumeTypeEnum.Proportion),
            },
            {
              name: '篮子标的csv模板(按数量).csv',
              data: getExportData(BasketVolumeTypeEnum.Quantity),
            },
          ],
          search: {
            keys: ['instrument_id', 'exchange_id', 'instrumentName'],
          },
          headers: ['instrument_id', 'exchange_id', 'volume', 'rate'],
          default: [],
          noDivider: true,
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
