import { DealTradingTableHooks } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingTableHook';
import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { getTradingDataSortKey } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { FundTransTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
const { t } = VueI18n.global;

export const getColumns = (
  kfLocation: KungfuApi.KfLocation,
  sorter: (
    dataIndex: string,
  ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
  marginSorter: (
    dataIndex: string,
  ) => (a: KungfuApi.KfConfig, b: KungfuApi.KfConfig) => number,
  isShowAssetMargin: boolean,
): AntTableColumns =>
  (globalThis.HookKeeper.getHooks().dealTradingTable as DealTradingTableHooks)
    .trigger(kfLocation, 'td')
    .getColumns<AntTableColumn>([
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

      ...(isShowAssetMargin
        ? [
            {
              title: t('tdConfig.avail_margin'),
              dataIndex: 'avail_margin',
              align: 'right',
              sorter: {
                compare: marginSorter('avail_margin'),
              },
              width: 110,
            },
            {
              title: t('tdConfig.cash_debt'),
              dataIndex: 'cash_debt',
              align: 'right',
              sorter: {
                compare: marginSorter('cash_debt'),
              },
              width: 110,
            },
            {
              title: t('tdConfig.total_asset'),
              dataIndex: 'total_asset',
              align: 'right',
              sorter: {
                compare: marginSorter('total_asset'),
              },
              width: 110,
            },
          ]
        : []),

      {
        title: t('tdConfig.actions'),
        dataIndex: 'actions',
        align: 'right',
        width: 140,
        fixed: 'right',
      },
    ]);

const orderSortKey = getTradingDataSortKey('Order');
const tradeSortKey = getTradingDataSortKey('Trade');
const positionSortKey = getTradingDataSortKey('Position');

export const categoryRegisterConfig: DealTradingDataGetter = {
  category: 'tdGroup',
  commonData: {
    name: t('tdConfig.td_group'),
    color: '#FAAD14',
  },
  order: {
    getter(watcher, orders, kfLocation: KungfuApi.KfExtraLocation) {
      const { children } = kfLocation;
      const tdList = (children || []) as KungfuApi.KfConfig[];
      const locationUids = tdList.map((item) => item.location_uid);
      return orders.sort(orderSortKey).filter((item) => {
        return locationUids.indexOf(item.source) !== -1;
      });
    },
  },
  trade: {
    getter(watcher, trades, kfLocation: KungfuApi.KfExtraLocation) {
      const { children } = kfLocation;
      const tdList = (children || []) as KungfuApi.KfConfig[];
      const locationUids = tdList.map((item) => item.location_uid);
      return trades.sort(tradeSortKey).filter((item) => {
        return locationUids.indexOf(item.source) !== -1;
      });
    },
  },
  position: {
    getter(watcher, position, kfLocation: KungfuApi.KfExtraLocation) {
      const { children } = kfLocation;
      const tdList = (children || []) as KungfuApi.KfConfig[];
      const locationUids = tdList.map((item) => item.location_uid);
      return position
        .nofilter('volume', BigInt(0))
        .filter('ledger_category', LedgerCategoryEnum.td)
        .sort(positionSortKey)
        .reverse()
        .filter((item) => {
          return locationUids.indexOf(item.holder_uid) !== -1;
        });
    },
  },
};

export const getFundTransKey = (type: FundTransTypeEnum | null): string => {
  if (type === FundTransTypeEnum.BetweenNodes) {
    return 'FundTransBetweenNodes';
  } else if (type === FundTransTypeEnum.TrancIn) {
    return 'FundTransIn';
  } else if (type === FundTransTypeEnum.TrancOut) {
    return 'FundTransOut';
  } else {
    return 'FundTrans';
  }
};
