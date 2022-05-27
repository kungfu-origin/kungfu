import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const buildSorter =
  (dataIndex: keyof KungfuApi.TradeResolved) =>
  (a: KungfuApi.TradeResolved, b: KungfuApi.TradeResolved) =>
    +Number(a[dataIndex]) - +Number(b[dataIndex]);

const buildStrSorter =
  (dataIndex: keyof KungfuApi.TradeResolved) =>
  (a: KungfuApi.TradeResolved, b: KungfuApi.TradeResolved) =>
    a[dataIndex].toString().localeCompare(b[dataIndex].toString());

export const getColumns = (
  category: KfCategoryTypes,
  isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
  {
    type: 'string',
    name: t('tradeConfig.trade_time_resolved'),
    dataIndex: 'trade_time_resolved',
    width: isHistory ? 160 : 120,
    sorter: buildSorter('trade_time'),
  },
  {
    type: 'string',
    name: t('tradeConfig.kf_time_resolved'),
    dataIndex: 'kf_time_resovlved',
    width: isHistory ? 160 : 120,
    sorter: buildSorter('trade_time'),
  },
  {
    type: 'string',
    name: t('tradeConfig.instrument_id'),
    dataIndex: 'instrument_id',
    sorter: buildStrSorter('instrument_id'),
    width: 60,
  },
  {
    type: 'string',
    name: '',
    dataIndex: 'side',
    width: 40,
  },
  {
    type: 'string',
    name: '',
    dataIndex: 'offset',
    width: 40,
  },
  {
    type: 'number',
    name: t('tradeConfig.price'),
    dataIndex: 'price',
    width: 120,
    sorter: buildSorter('price'),
  },
  {
    type: 'number',
    name: t('tradeConfig.volume'),
    dataIndex: 'volume',
    width: 60,
    sorter: buildSorter('volume'),
  },
  {
    type: 'number',
    name: t('tradeConfig.latency_trade'),
    dataIndex: 'latency_trade',
    width: 90,
    sorter: (a: KungfuApi.TradeResolved, b: KungfuApi.TradeResolved) =>
      +a.latency_trade - +b.latency_trade,
  },
  {
    name:
      category == 'td'
        ? t('orderConfig.dest_uname')
        : t('orderConfig.source_uname'),
    dataIndex: category == 'td' ? 'dest_uname' : 'source_uname',
    sorter: buildStrSorter(category == 'td' ? 'dest_uname' : 'source_uname'),
    flex: 1,
  },
  ...(isTdStrategyCategory(category)
    ? []
    : [
        {
          name: t('orderConfig.dest_uname'),
          dataIndex: 'dest_uname',
          sorter: buildStrSorter('dest_uname'),
          flex: 1,
        },
      ]),
];

export const statisColums: AntTableColumns = [
  {
    title: t('tradeConfig.instrument'),
    dataIndex: 'instrumentId_exchangeId',
  },
  {
    title: '',
    dataIndex: 'side',
  },
  {
    title: '',
    dataIndex: 'offset',
  },
  {
    title: t('tradeConfig.mean_price'),
    dataIndex: 'mean',
  },
  {
    title: t('tradeConfig.max_price'),
    dataIndex: 'max',
  },
  {
    title: t('tradeConfig.min_price'),
    dataIndex: 'min',
  },
  {
    title: t('tradeConfig.volume'),
    dataIndex: 'volume',
  },
];
