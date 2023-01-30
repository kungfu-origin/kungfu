import {
  KfCategory,
  UnknownKfCategory,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  KfCategoryEnum,
  KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { dealTradingData } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

export interface DealTradingDataGetter {
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

const DefaultTdDealTrdaingDataHook = {
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
};

const DefaultStrategyDealTrdaingDataHook = {
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
};

const DefaultUnkownDealTrdaingDataHook = {
  category: 'Unknown',
  commonData: UnknownKfCategory,

  order: {
    getter: (
      // eslint-disable-next-line
      ...args: [
        watcher: KungfuApi.Watcher,
        orders: KungfuApi.DataTable<KungfuApi.Order>,
        kfLocation: KungfuApi.KfLocation,
      ]
    ) => [] as KungfuApi.Order[],
  },

  trade: {
    getter: (
      // eslint-disable-next-line
      ...args: [
        watcher: KungfuApi.Watcher,
        trades: KungfuApi.DataTable<KungfuApi.Trade>,
        kfLocation: KungfuApi.KfLocation,
      ]
    ) => [] as KungfuApi.Trade[],
  },

  position: {
    getter: (
      // eslint-disable-next-line
      ...args: [
        watcher: KungfuApi.Watcher,
        positions: KungfuApi.DataTable<KungfuApi.Position>,
        kfLocation: KungfuApi.KfLocation,
      ]
    ) => [] as KungfuApi.Position[],
  },
};

export class DealTradingDataHooks {
  hooks: Record<string, DealTradingDataGetter>;
  constructor() {
    this.hooks = new Proxy(
      {
        'td_*_*': DefaultTdDealTrdaingDataHook,
        'strategy_*_*': DefaultStrategyDealTrdaingDataHook,
      },
      {
        get(target: Record<string, DealTradingDataGetter>, prop: string) {
          const locationPairs = prop.split('_');
          if (locationPairs.length != 3) {
            console.warn(`Invalid hook key: ${prop}`);
            return [];
          }

          const [category, group, name] = prop.split('_');
          if (target[`${category}_${group}_${name}`]) {
            return target[`${category}_${group}_${name}`];
          } else if (target[`${category}_*_${name}`]) {
            return target[`${category}_*_${name}`];
          } else if (target[`${category}_${group}_*`]) {
            return target[`${category}_${group}_*`];
          } else if (target[`${category}_*_*`]) {
            return target[`${category}_*_*`];
          }

          // eslint-disable-next-line
          return (key: string) => DefaultUnkownDealTrdaingDataHook;
        },

        set(
          target: Record<string, DealTradingDataGetter>,
          prop: string,
          value: DealTradingDataGetter,
        ) {
          if (Reflect.has(target, prop)) {
            console.warn(`DealTradingData hook ${prop} already exists`);
            return true;
          }

          console.log(`DealTradingData hook ${prop} register success`);
          Reflect.set(target, prop, value);
          return true;
        },
      },
    );
  }

  register(
    kfLocation: KungfuApi.DerivedKfLocation,
    getter: DealTradingDataGetter,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, getter);
  }

  trigger(
    watcher: KungfuApi.Watcher | null,
    kfLocation: KungfuApi.DerivedKfLocation,
    dataSource: KungfuApi.TradingDataTable,
    tradingDataType: 'order' | 'trade' | 'position',
  ) {
    if (!watcher) {
      console.warn('Watcher is NULL');
      return [];
    }

    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    const getter = Reflect.get(this.hooks, key)[tradingDataType].getter;
    return getter(
      watcher,
      dataSource as KungfuApi.MergedTradingDataTable,
      kfLocation,
    );
  }

  getCategoryMap(): Record<string, KungfuApi.KfTradeValueCommonData> {
    return Object.keys(this.hooks).reduce((pre, key) => {
      const [category, group, name] = key.split('_');
      if (group === name && group === '*') {
        pre[category] = Reflect.get(this.hooks, key).commonData;
      }
      return pre;
    }, {});
  }
}
