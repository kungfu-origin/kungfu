import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { KfCategoryRegisterProps } from '@renderer/assets/methods/uiExtraLocationUtils';

export const getColumns = (
    sorter: (
        dataIndex: string,
    ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
): AntTableColumns => [
    {
        title: '账户(组)',
        dataIndex: 'name',
        align: 'left',
        width: 180,
        fixed: 'left',
    },
    {
        title: '备注',
        dataIndex: 'accountName',
        align: 'left',
        width: 80,
        fixed: 'left',
    },
    {
        title: '状态',
        dataIndex: 'stateStatus',
        align: 'left',
        width: 80,
        fixed: 'left',
    },
    {
        title: '进程',
        dataIndex: 'processStatus',
        align: 'center',
        width: 60,
        fixed: 'left',
    },
    {
        title: '浮动盈亏',
        dataIndex: 'unrealizedPnl',
        align: 'right',
        sorter: {
            compare: sorter('unrealized_pnl'),
        },
        width: 110,
    },
    {
        title: '市值',
        dataIndex: 'marketValue',
        align: 'right',
        sorter: {
            compare: sorter('market_value'),
        },
        width: 110,
    },
    {
        title: '保证金',
        dataIndex: 'margin',
        align: 'right',
        sorter: {
            compare: sorter('margin'),
        },
        width: 110,
    },
    {
        title: '可用资金',
        dataIndex: 'avail',
        align: 'right',
        sorter: {
            compare: sorter('avail'),
        },
        width: 110,
    },
    {
        title: '操作',
        dataIndex: 'actions',
        align: 'right',
        width: 140,
        fixed: 'right',
    },
];

export const categoryRegisterConfig: KfCategoryRegisterProps = {
    name: 'tdGroup',
    commonData: {
        name: '账户分组',
        color: '#FAAD14',
    },
    order: {
        getter(orders, kfLocation: KungfuApi.KfExtraLocation) {
            const { children } = kfLocation;
            const tdList = (children || []) as KungfuApi.KfConfig[];
            const locationUids = tdList.map((item) => item.location_uid);
            return orders.sort('update_time').filter((item) => {
                return locationUids.indexOf(item.source) !== -1;
            });
        },
    },
    trade: {
        getter(trades, kfLocation: KungfuApi.KfExtraLocation) {
            const { children } = kfLocation;
            const tdList = (children || []) as KungfuApi.KfConfig[];
            const locationUids = tdList.map((item) => item.location_uid);
            return trades.sort('trade_time').filter((item) => {
                return locationUids.indexOf(item.source) !== -1;
            });
        },
    },
    position: {
        getter(position, kfLocation: KungfuApi.KfExtraLocation) {
            const { children } = kfLocation;
            const tdList = (children || []) as KungfuApi.KfConfig[];
            const locationUids = tdList.map((item) => item.location_uid);
            return position
                .nofilter('volume', BigInt(0))
                .filter('ledger_category', LedgerCategoryEnum.td)
                .sort('instrument_id')
                .reverse()
                .filter((item) => {
                    return locationUids.indexOf(item.holder_uid) !== -1;
                });
        },
    },
};
