import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const buildSorter =
  (dataIndex: keyof KungfuApi.Position) =>
  (a: KungfuApi.Position, b: KungfuApi.Position) =>
    +Number(a[dataIndex]) - +Number(b[dataIndex]);

const buildStrSorter =
  (dataIndex: keyof KungfuApi.PositionResolved) =>
  (a: KungfuApi.PositionResolved, b: KungfuApi.PositionResolved) =>
    a[dataIndex].toString().localeCompare(b[dataIndex].toString());

export const getColumns = (
  category: KfCategoryTypes,
): KfTradingDataTableHeaderConfig[] => [
  {
    type: 'string',
    name: '代码',
    dataIndex: 'instrument_id_resolved',
    sorter: buildStrSorter('instrument_id_resolved'),
    width: 120,
  },
  ...(isTdStrategyCategory(category)
    ? []
    : [
        {
          name: '持有账户',
          dataIndex: 'account_id_resolved',
          sorter: buildStrSorter('account_id_resolved'),
          width: 120,
        },
      ]),
  {
    type: 'string',
    name: '',
    dataIndex: 'direction',
    sorter: buildSorter('direction'),
    width: 40,
  },
  {
    type: 'number',
    name: '昨',
    dataIndex: 'yesterday_volume',
    flex: 1,
    sorter: buildSorter('yesterday_volume'),
  },
  {
    type: 'number',
    name: '今',
    dataIndex: 'today_volume',
    flex: 1,
    sorter: (a: KungfuApi.Position, b: KungfuApi.Position) => {
      const deltaA = a.volume - a.yesterday_volume;
      const deltaB = b.volume - b.yesterday_volume;
      return +Number(deltaA) - +Number(deltaB);
    },
  },
  {
    type: 'number',
    name: '总',
    dataIndex: 'volume',
    flex: 1,
    sorter: buildSorter('yesterday_volume'),
  },
  {
    type: 'number',
    name: '开仓均价',
    dataIndex: 'avg_open_price',
    flex: 1.2,
    sorter: buildSorter('avg_open_price'),
  },
  {
    type: 'number',
    name: '最新价',
    dataIndex: 'last_price',
    flex: 1.5,
    sorter: buildSorter('last_price'),
  },
  {
    type: 'number',
    name: '浮动盈亏',
    dataIndex: 'unrealized_pnl',
    flex: 1.5,
    sorter: buildSorter('unrealized_pnl'),
  },
];
