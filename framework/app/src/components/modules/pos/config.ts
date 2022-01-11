import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { isTdStrategyCategory } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

export const getColumns = (
    category: KfCategoryTypes,
): KfTradingDataTableHeaderConfig[] => [
    {
        type: 'string',
        name: '代码',
        dataIndex: 'instrument_id',
        width: 100,
    },
    {
        type: 'string',
        name: '',
        dataIndex: 'direction',
        width: 40,
    },
    {
        type: 'number',
        name: '昨',
        dataIndex: 'yesterday_volume',
        flex: 1,
    },
    {
        type: 'number',
        name: '今',
        dataIndex: 'today_volume',
        flex: 1,
    },
    {
        type: 'number',
        name: '总',
        dataIndex: 'volume',
        flex: 1,
    },
    {
        type: 'number',
        name: '开仓均价',
        dataIndex: 'avg_open_price',
        flex: 1.2,
    },
    {
        type: 'number',
        name: '总成本',
        dataIndex: 'total_price',
        flex: 1.5,
    },
    {
        type: 'number',
        name: '浮动盈亏',
        dataIndex: 'unrealized_pnl',
        flex: 1.5,
    },
    ...(isTdStrategyCategory(category)
        ? []
        : [
              {
                  name: '持有账户',
                  dataIndex: 'holder_uid',
                  flex: 1,
              },
          ]),
];
