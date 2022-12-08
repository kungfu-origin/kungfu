interface KfTradingDataTableHeaderConfig {
  name: string;
  dataIndex: string;
  width?: number;
  flex?: number;
  type?:
    | 'number'
    | 'string'
    | 'source'
    | 'nanoTime'
    | 'exchange'
    | 'offset'
    | 'side'
    | 'priceType'
    | 'direction'
    | 'actions';
  sorter?: (a: any, b: any) => number;
}

interface AntTableColumn {
  title: string;
  dataIndex: string;
  key?: string;
  width?: number | string;
  minWidth?: number | string;
  sorter?: boolean | { compare: (a: any, b: any) => number };
  align?: string;
  fixed?: string;
}

export type TradingTableTypes = 'position' | 'trade' | 'order' | 'td';

export type TradingTableColumnsTypes =
  | AntTableColumn
  | KfTradingDataTableHeaderConfig;

export type TradingTableDealer = <T extends TradingTableColumnsTypes>(
  columns: T[],
  kfLocation: KungfuApi.DerivedKfLocation,
  tableType: TradingTableTypes,
) => T[];

export type TradingTableDealerMap = Partial<
  Record<TradingTableTypes, TradingTableDealer[]>
>;

const DefaultUnkownTrdaingTableDealerMap = {};

export class DealTradingTableHooks {
  hooks: Record<string, TradingTableDealerMap>;
  constructor() {
    this.hooks = new Proxy(
      {},
      {
        get(target: Record<string, TradingTableDealerMap>, prop: string) {
          const locationPairs = prop.split('_');
          if (locationPairs.length != 3) {
            console.warn(`Invalid hook key: ${prop}`);
            return DefaultUnkownTrdaingTableDealerMap;
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

          return DefaultUnkownTrdaingTableDealerMap;
        },

        set(
          target: Record<string, TradingTableDealerMap>,
          prop: string,
          value: TradingTableDealerMap,
        ) {
          let exsitedDealerMap = {};
          if (Reflect.has(target, prop)) {
            exsitedDealerMap = Reflect.get(target, prop);
          }

          Object.keys(value).forEach((key) => {
            if (exsitedDealerMap[key] && Array.isArray(exsitedDealerMap[key])) {
              exsitedDealerMap[key].push(value[key]);
            } else {
              exsitedDealerMap[key] = [value[key]];
            }
          });

          Reflect.set(target, prop, exsitedDealerMap);
          console.log(`DealTradingTable hook ${prop} register success`);
          return true;
        },
      },
    );
  }

  register(
    kfLocation: KungfuApi.DerivedKfLocation,
    tradingTableType: TradingTableTypes,
    dealer: TradingTableDealer,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, { [tradingTableType]: dealer });
  }

  trigger<T extends TradingTableColumnsTypes>(
    kfLocation: KungfuApi.DerivedKfLocation,
    tradingTableType: TradingTableTypes,
    columns: T[],
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    const dealers = (Reflect.get(this.hooks, key) as TradingTableDealerMap)[
      tradingTableType
    ];

    if (dealers?.length) {
      console.log(
        `DealTradingTable hook ${key} ${tradingTableType} trigger success`,
      );

      return dealers.reduce(
        (dealedColumns, dealer) =>
          dealer<T>(dealedColumns, kfLocation, tradingTableType),
        columns,
      );
    }

    return columns;
  }
}
