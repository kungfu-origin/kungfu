import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const buildSorter =
  (dataIndex: keyof KungfuApi.OrderResolved) =>
  (a: KungfuApi.OrderResolved, b: KungfuApi.OrderResolved) =>
    +Number(a[dataIndex]) - +Number(b[dataIndex]);

const buildStrSorter =
  (dataIndex: keyof KungfuApi.OrderResolved) =>
  (a: KungfuApi.OrderResolved, b: KungfuApi.OrderResolved) =>
    a[dataIndex].toString().localeCompare(b[dataIndex].toString());

export const getColumns = (
  category: KfCategoryTypes,
  isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
  {
    type: 'string',
    name: t('orderConfig.update_time'),
    dataIndex: 'update_time_resolved',
    width: isHistory ? 160 : 120,
    sorter: buildSorter('update_time'),
  },
  {
    type: 'string',
    name: t('orderConfig.instrument_id'),
    dataIndex: 'instrument_id',
    sorter: buildStrSorter('instrument_id'),
    width: 80,
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
    name: t('orderConfig.limit_price'),
    dataIndex: 'limit_price',
    width: 120,
    sorter: buildSorter('limit_price'),
  },
  {
    type: 'number',
    name: `${t('orderConfig.clinch')}/${t('orderConfig.all')}`,
    dataIndex: 'volume_traded',
    width: 120,
    sorter: (a: KungfuApi.OrderResolved, b: KungfuApi.OrderResolved) => {
      return +Number(b.volume_left) - +Number(a.volume_left);
    },
  },
  {
    type: 'string',
    name: t('orderConfig.order_status'),
    dataIndex: 'status_uname',
    width: 120,
  },
  {
    type: 'number',
    name: t('orderConfig.latency_system'),
    dataIndex: 'latency_system',
    width: 90,
    sorter: buildSorter('latency_system'),
  },
  {
    type: 'number',
    name: t('orderConfig.latency_network'),
    dataIndex: 'latency_network',
    width: 90,
    sorter: buildSorter('latency_network'),
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
  ...(isHistory
    ? []
    : [
        {
          name: '',
          dataIndex: 'actions',
          width: 60,
        },
      ]),
];

export const statisColums: AntTableColumns = [
  {
    title: t('tradingConfig.instrument'),
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
    title: t('orderConfig.mean'),
    dataIndex: 'mean',
  },
  {
    title: t('orderConfig.max'),
    dataIndex: 'max',
  },
  {
    title: t('orderConfig.min'),
    dataIndex: 'min',
  },
  {
    title: `${t('orderConfig.volume')}(${t('orderConfig.completed')}/${t(
      'orderConfig.all',
    )})`,
    dataIndex: 'volume',
  },
];
