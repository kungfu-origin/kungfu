import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { KfCategoryRegisterProps } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getColumns = (
  sorter: (
    dataIndex: string,
  ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
): AntTableColumns => [
  {
    title: t('tdConfig.account_name'),
    dataIndex: 'name',
    align: 'left',
    width: 180,
    minWidth: 180,
    fixed: 'left',
  },
  {
    title: t('tdConfig.account_ps'),
    dataIndex: 'accountName',
    align: 'left',
    width: 80,
    fixed: 'left',
  },
  {
    title: t('tdConfig.state_status'),
    dataIndex: 'stateStatus',
    align: 'left',
    width: 80,
    fixed: 'left',
  },
  {
    title: t('tdConfig.process_status'),
    dataIndex: 'processStatus',
    align: 'center',
    width: 60,
    fixed: 'left',
  },
  {
    title: t('tdConfig.unrealized_pnl'),
    dataIndex: 'unrealizedPnl',
    align: 'right',
    sorter: {
      compare: sorter('unrealized_pnl'),
    },
    width: 110,
  },
  {
    title: t('tdConfig.marked_value'),
    dataIndex: 'marketValue',
    align: 'right',
    sorter: {
      compare: sorter('market_value'),
    },
    width: 110,
  },
  {
    title: t('tdConfig.margin'),
    dataIndex: 'margin',
    align: 'right',
    sorter: {
      compare: sorter('margin'),
    },
    width: 110,
  },
  {
    title: t('tdConfig.avail_money'),
    dataIndex: 'avail',
    align: 'right',
    sorter: {
      compare: sorter('avail'),
    },
    width: 110,
  },
  {
    title: t('tdConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 140,
    fixed: 'right',
  },
];

export const categoryRegisterConfig: KfCategoryRegisterProps = {
  name: 'tdGroup',
  commonData: {
    name: t('tdConfig.td_group'),
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
