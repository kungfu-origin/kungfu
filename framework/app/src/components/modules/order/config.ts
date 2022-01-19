import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const buildSorter =
    (dataIndex: keyof KungfuApi.OrderResolved) =>
    (a: KungfuApi.OrderResolved, b: KungfuApi.OrderResolved) =>
        +Number(a[dataIndex]) - +Number(b[dataIndex]);

export const getColumns = (
    category: KfCategoryTypes,
    isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
    {
        type: 'string',
        name: '更新时间',
        dataIndex: 'update_time',
        width: isHistory ? 160 : 120,
        sorter: buildSorter('update_time'),
    },
    {
        type: 'string',
        name: '代码',
        dataIndex: 'instrument_id',
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
        name: '委托价',
        dataIndex: 'limit_price',
        width: 120,
        sorter: buildSorter('limit_price'),
    },
    {
        type: 'number',
        name: '已成交/全部',
        dataIndex: 'volume_traded',
        width: 120,
        sorter: (a: KungfuApi.OrderResolved, b: KungfuApi.OrderResolved) => {
            return +Number(b.volume_left) - +Number(a.volume_left);
        },
    },
    {
        type: 'string',
        name: '订单状态',
        dataIndex: 'status',
        width: 120,
    },
    {
        type: 'number',
        name: '系统延迟μs',
        dataIndex: 'latency_system',
        width: 90,
        sorter: buildSorter('latency_system'),
    },
    {
        type: 'number',
        name: '网络延迟μs',
        dataIndex: 'latency_network',
        width: 90,
        sorter: buildSorter('latency_network'),
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
        title: '标的',
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
        title: '委托均价',
        dataIndex: 'mean',
    },
    {
        title: '最高委托价',
        dataIndex: 'max',
    },
    {
        title: '最低委托价',
        dataIndex: 'min',
    },
];
