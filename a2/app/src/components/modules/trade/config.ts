import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';

export const getColumns = (
    moduleType: KfCategoryTypes,
    isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
    {
        type: 'string',
        name: '成交时间',
        dataIndex: 'trade_time',
        width: isHistory ? 140 : 100,
    },
    {
        type: 'string',
        name: '系统时间',
        dataIndex: 'kf_time',
        width: isHistory ? 140 : 100,
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
    },
    {
        type: 'number',
        name: '成交量',
        dataIndex: 'volume',
        width: 60,
    },
    {
        type: 'number',
        name: '成交延迟(μs)',
        dataIndex: 'latency_trade',
        width: 90,
    },
    {
        name: moduleType == 'td' ? '下单源' : '目标账户',
        dataIndex: moduleType == 'td' ? 'dest' : 'source',
    },
];
