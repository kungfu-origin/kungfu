import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings';

export const getColumns = (
    moduleType: KfCategoryTypes,
    isHistory = false,
): KfTradingDataTableHeaderConfig[] => [
    {
        type: 'string',
        name: '更新时间',
        dataIndex: 'update_time',
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
        name: '委托价',
        dataIndex: 'limit_price',
        width: 120,
    },
    {
        type: 'string',
        name: '已成交/全部',
        dataIndex: 'volume_traded',
        width: 120,
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
    },
    {
        type: 'number',
        name: '网络延迟μs',
        dataIndex: 'latency_network',
        width: 90,
    },
    {
        name: moduleType == 'td' ? '下单源' : '目标账户',
        dataIndex: moduleType == 'td' ? 'dest' : 'source',
    },
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
