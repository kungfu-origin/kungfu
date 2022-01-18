import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const buildSorter =
    (dataIndex: keyof KungfuApi.TradeResolved) =>
    (a: KungfuApi.TradeResolved, b: KungfuApi.TradeResolved) =>
        +Number(a[dataIndex]) - +Number(b[dataIndex]);

export const getColumns = (
    category: KfCategoryTypes,
    isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
    {
        type: 'string',
        name: '成交时间',
        dataIndex: 'trade_time',
        width: isHistory ? 160 : 120,
        sorter: buildSorter('trade_time'),
    },
    {
        type: 'string',
        name: '系统时间',
        dataIndex: 'kf_time',
        width: isHistory ? 160 : 120,
        sorter: buildSorter('trade_time'),
    },
    {
        type: 'string',
        name: '代码',
        dataIndex: 'instrument_id',
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
        name: '成交价',
        dataIndex: 'price',
        width: 120,
        sorter: buildSorter('price'),
    },
    {
        type: 'number',
        name: '成交量',
        dataIndex: 'volume',
        width: 60,
        sorter: buildSorter('volume'),
    },
    {
        type: 'number',
        name: '成交延迟(μs)',
        dataIndex: 'latency_trade',
        width: 90,
        sorter: (a: KungfuApi.TradeResolved, b: KungfuApi.TradeResolved) =>
            +a.latency_trade - +b.latency_trade,
    },
    {
        name: category == 'td' ? '下单源' : '目标账户',
        dataIndex: category == 'td' ? 'dest' : 'source',
        flex: 1,
    },
    ...(isTdStrategyCategory(category)
        ? []
        : [
              {
                  name: '下单源',
                  dataIndex: 'dest',
                  flex: 1,
              },
          ]),
];
