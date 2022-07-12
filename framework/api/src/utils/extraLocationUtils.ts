import { KfCategory } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  KfCategoryEnum,
  KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { dealTradingData } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const kfCategoryGetters: Record<string, KfCategoryRegisterProps> = {
  td: {
    category: 'td',
    commonData: KfCategory[KfCategoryEnum.td],
    order: {
      getter: (
        watcher: KungfuApi.Watcher,
        orders: KungfuApi.DataTable<KungfuApi.Order>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Order>(
          watcher,
          orders,
          'Order',
          kfLocation,
        ) as KungfuApi.Order[];
      },
    },
    trade: {
      getter: (
        watcher: KungfuApi.Watcher,
        trades: KungfuApi.DataTable<KungfuApi.Trade>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Trade>(
          watcher,
          trades,
          'Trade',
          kfLocation,
        ) as KungfuApi.Trade[];
      },
    },
    position: {
      getter: (
        watcher: KungfuApi.Watcher,
        positions: KungfuApi.DataTable<KungfuApi.Position>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Position>(
          watcher,
          positions,
          'Position',
          kfLocation,
        ) as KungfuApi.Position[];
      },
    },
  },

  strategy: {
    category: 'strategy',
    commonData: KfCategory[KfCategoryEnum.strategy],
    order: {
      getter: (
        watcher: KungfuApi.Watcher,
        orders: KungfuApi.DataTable<KungfuApi.Order>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Order>(
          watcher,
          orders,
          'Order',
          kfLocation,
        ) as KungfuApi.Order[];
      },
    },
    trade: {
      getter: (
        watcher: KungfuApi.Watcher,
        trades: KungfuApi.DataTable<KungfuApi.Trade>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Trade>(
          watcher,
          trades,
          'Trade',
          kfLocation,
        ) as KungfuApi.Trade[];
      },
    },
    position: {
      getter: (
        watcher: KungfuApi.Watcher,
        positions: KungfuApi.DataTable<KungfuApi.Position>,
        kfLocation: KungfuApi.KfLocation,
      ) => {
        return dealTradingData<KungfuApi.Position>(
          watcher,
          positions,
          'Position',
          kfLocation,
        ) as KungfuApi.Position[];
      },
    },
  },
};

export interface KfCategoryRegisterProps {
  category: KfCategoryTypes | string;
  commonData: KungfuApi.KfTradeValueCommonData;

  order: {
    getter: (
      watcher: KungfuApi.Watcher,
      orders: KungfuApi.DataTable<KungfuApi.Order>,
      kfLocation: KungfuApi.KfLocation,
    ) => KungfuApi.Order[];
  };

  trade: {
    getter: (
      watcher: KungfuApi.Watcher,
      trades: KungfuApi.DataTable<KungfuApi.Trade>,
      kfLocation: KungfuApi.KfLocation,
    ) => KungfuApi.Trade[];
  };

  position: {
    getter: (
      watcher: KungfuApi.Watcher,
      positions: KungfuApi.DataTable<KungfuApi.Position>,
      kfLocation: KungfuApi.KfLocation,
    ) => KungfuApi.Position[];
  };
}

export class GlobalCategoryRegister {
  globalRegistedCategories: Record<string, KfCategoryRegisterProps>;

  constructor() {
    this.globalRegistedCategories = {
      ...kfCategoryGetters,
    };
  }

  register(props: KfCategoryRegisterProps): void {
    if (this.globalRegistedCategories[props.category]) {
      return;
    }

    this.globalRegistedCategories[props.category] = props;
  }

  getExtraCategoryMap(): Record<string, KungfuApi.KfTradeValueCommonData> {
    return Object.keys(this.globalRegistedCategories).reduce(
      (extraCategoryData, key) => {
        extraCategoryData[key] = this.globalRegistedCategories[key].commonData;
        return extraCategoryData;
      },
      {} as Record<string, KungfuApi.KfTradeValueCommonData>,
    );
  }
}

export const useExtraCategory = (): {
  getTradingDataByLocation: (
    targetCategoryProps: KfCategoryRegisterProps | null,
    dataSource: KungfuApi.TradingDataTable,
    kfLocation: KungfuApi.KfLocation,
    watcher: KungfuApi.Watcher | null,
    tradingDataType: 'order' | 'trade' | 'position',
  ) => KungfuApi.TradingDataItem[];
} => {
  const getTradingDataByLocation = (
    targetCategoryProps: KfCategoryRegisterProps | null,
    dataSource: KungfuApi.TradingDataTable,
    kfLocation: KungfuApi.KfLocation,
    watcher: KungfuApi.Watcher | null,
    tradingDataType: 'order' | 'trade' | 'position',
  ): KungfuApi.TradingDataItem[] => {
    if (!targetCategoryProps) {
      return [];
    }

    if (!watcher) {
      console.warn('Watcher is NULL');
      return [];
    }

    const defaulFuncData = {
      getter: (
        watcher: KungfuApi.Watcher,
        dataSource: KungfuApi.TradingDataTable,
        kfLocation: KungfuApi.KfLocation,
      ) => [] as KungfuApi.TradingDataItem[],
    };
    const dealDataFuncData =
      targetCategoryProps[tradingDataType] || defaulFuncData;
    const dealDataFunc = dealDataFuncData.getter;

    return dealDataFunc(
      watcher,
      dataSource as KungfuApi.MergedTradingDataTable,
      kfLocation,
    );
  };

  return {
    getTradingDataByLocation,
  };
};
