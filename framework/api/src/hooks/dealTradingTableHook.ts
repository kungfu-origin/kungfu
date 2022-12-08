export interface KfTradingDataTableHeaderConfig {
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
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  sorter?: (a: any, b: any) => number;
}

export interface AntTableColumn {
  title: string;
  dataIndex: string;
  key?: string;
  width?: number | string;
  minWidth?: number | string;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  sorter?: boolean | { compare: (a: any, b: any) => number };
  align?: string;
  fixed?: string;
}

export type TradingTableTypes =
  | 'position'
  | 'trade'
  | 'order'
  | 'td'
  | 'unkown';

export type TradingTableColumnsTypes =
  | AntTableColumn
  | KfTradingDataTableHeaderConfig;

export type TradingDataTypes =
  | KungfuApi.PositionResolved
  | KungfuApi.TradeResolved
  | KungfuApi.OrderResolved;

export class TradingTableDealer {
  locationKey: string;
  tradingTableType: TradingTableTypes;
  columnsDealers: Array<
    <ColumnsType extends TradingTableColumnsTypes>(
      columns: ColumnsType[],
    ) => ColumnsType[]
  >;
  dataResolvers: Array<
    <DataType extends TradingDataTypes>(datas: DataType[]) => DataType[]
  >;

  constructor(loactionKey: string, tradingTableType: TradingTableTypes) {
    this.locationKey = loactionKey;
    this.tradingTableType = tradingTableType;
    this.columnsDealers = [];
    this.dataResolvers = [];
  }

  getColumns<ColumnsType extends TradingTableColumnsTypes>(
    columns: ColumnsType[],
  ) {
    if (this.columnsDealers.length) {
      console.log(
        `DealTradingTable hook ${this.locationKey} ${this.tradingTableType} trigger getColumns success`,
      );

      return this.columnsDealers.reduce(
        (dealedColumns, dealer) => dealer(dealedColumns),
        columns,
      );
    }

    return columns;
  }

  resolveData<DataType extends TradingDataTypes>(datas: DataType[]) {
    if (this.dataResolvers.length) {
      console.log(
        `DealTradingTable hook ${this.locationKey} ${this.tradingTableType} trigger resolveDatas success`,
      );

      return this.dataResolvers.reduce(
        (resolvedDatas, resolver) => resolver(resolvedDatas),
        datas,
      );
    }

    return datas;
  }
}

export type TradingTableDealerMap = Partial<
  Record<TradingTableTypes, TradingTableDealer>
>;

const DefaultUnkownTrdaingTableDealerMap = {};

const DefaultTradingTableDealer = new TradingTableDealer('unkown', 'unkown');

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
          value: Record<
            TradingTableTypes,
            {
              getColumns: <ColumnsType extends TradingTableColumnsTypes>(
                columns: ColumnsType[],
              ) => ColumnsType[];
              resolvedDatas: <DataType extends TradingDataTypes>(
                datas: DataType[],
              ) => DataType[];
            }
          >,
        ) {
          let exsitedDealerMap: TradingTableDealerMap = {};
          if (Reflect.has(target, prop)) {
            exsitedDealerMap = Reflect.get(target, prop);
          }

          Object.keys(value).forEach((key) => {
            const tradingTableType = key as TradingTableTypes;
            if (exsitedDealerMap[tradingTableType]) {
              value[tradingTableType].getColumns &&
                exsitedDealerMap[tradingTableType]?.columnsDealers.push(
                  value[tradingTableType].getColumns,
                );
              value[tradingTableType].resolvedDatas &&
                exsitedDealerMap[tradingTableType]?.dataResolvers.push(
                  value[tradingTableType].resolvedDatas,
                );
            } else {
              const newDealer = new TradingTableDealer(prop, tradingTableType);
              value[tradingTableType].getColumns &&
                newDealer.columnsDealers.push(
                  value[tradingTableType].getColumns,
                );
              value[tradingTableType].resolvedDatas &&
                newDealer.dataResolvers.push(
                  value[tradingTableType].resolvedDatas,
                );
              exsitedDealerMap[tradingTableType] = newDealer;
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
    dealer: {
      getColumns?: (
        columns: TradingTableColumnsTypes[],
      ) => TradingTableColumnsTypes[];
      resolvedDatas?: (datas: TradingDataTypes[]) => TradingDataTypes[];
    },
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, { [tradingTableType]: dealer });
  }

  trigger(
    kfLocation: KungfuApi.DerivedKfLocation,
    tradingTableType: TradingTableTypes,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    const dealer = (Reflect.get(this.hooks, key) as TradingTableDealerMap)[
      tradingTableType
    ];

    return dealer ?? DefaultTradingTableDealer;
  }
}
