declare const __resources: string;
declare const __python_version: string;
declare const __git_commit_version: string;
declare const __build_timestamp: number;

declare module NodeJS {
  interface Global {
    __resources: string;
    __kfResourcesPath: string;
  }

  interface Process {
    resourcesPath: string;
  }
}

declare module 'tail' {
  export class Tail {
    constructor(
      filePath: string,
      options: {
        follow?: boolean;
        fromBeginning?: boolean;
        nLines?: number;
      },
    );
    watch(): void;
    unwatch(): void;
    on(type: 'line', callback: (data: string) => void);
    on(type: 'error', callback: (err: Error) => void);
  }
}

declare namespace KungfuApi {
  import {
    BrokerStateStatusEnum,
    InstrumentTypeEnum,
    InstrumentTypes,
    PriceTypeEnum,
    SideEnum,
    OffsetEnum,
    HedgeFlagEnum,
    LedgerCategoryEnum,
    VolumeConditionEnum,
    TimeConditionEnum,
    CommissionModeEnum,
    DirectionEnum,
    OrderStatusEnum,
    KfCategoryEnum,
    KfCategoryTypes,
    KfUIExtLocatorTypes,
    KfModeEnum,
    KfModeTypes,
    OrderActionFlagEnum,
  } from './enums';
  import { Dayjs } from 'dayjs';

  export type AntInKungfuColorTypes =
    | 'default'
    | 'orange'
    | 'yellow'
    | 'pink'
    | 'red'
    | 'blue'
    | 'gray'
    | 'purple'
    | 'cyan'
    | 'green'
    | 'text'
    | '#FAAD14'
    | 'kf-color-running'
    | 'kf-color-waiting'
    | 'kf-color-error';

  export interface KfTradeValueCommonData {
    name: string;
    color?: AntInKungfuColorTypes;
    level?: number;
  }

  type AnyEnumKeysAsStrings<TEnumType> = keyof TEnumType;

  export type KfConfigItemSupportedTypes =
    | 'str'
    | 'password'
    | 'file' // string
    | 'files' // string[]
    | 'folder' // string
    | 'timePicker' //string
    | 'select'
    | 'bool'
    | 'int'
    | 'float'
    | 'percent'
    | 'side' // select - number
    | 'offset' // select - number
    | 'direction' // select - number
    | 'priceType' // select - number
    | 'hedgeFlag' // select - number
    | 'volumeCondition' // select - number
    | 'timeCondition' // select - number
    | 'commissionMode' // select - number
    | 'instrumentType' // select - number
    | 'td'
    | 'md'
    | 'strategy'
    | 'instrument'
    | 'instruments';

  export type KfConfigValue =
    | string
    | number
    | boolean
    | string[]
    | number[]
    | boolean[]
    | Dayjs;

  export interface KfSelectOption {
    value: string | number;
    label: string | number;
  }

  export interface KfConfigItem {
    key: string;
    name: string;
    type: KfConfigItemSupportedTypes;
    errMsg?: string;
    default?: KfConfigValue;
    required?: boolean;
    primary?: boolean;
    options?: KfSelectOption[];
    data?: KfSelectOption[];
    args?: Array<{ key: string | number; value: string | number }>; // process
    target?: string; // process
    tip?: string;
  }

  export interface KfExtOriginConfig {
    key: string;
    name: string;
    extPath: string;
    ui_config?: {
      position: KfUIExtLocatorTypes;
      components?:
        | {
            index: string;
          }
        | {
            entry: string;
            page: string;
          };
      daemon?: Record<string, string>;
    };
    config?: Record<
      string,
      {
        type?: Array<InstrumentTypes> | InstrumentTypes;
        settings: KfConfigItem[];
      }
    >;
  }

  interface KfExtConfig {
    name: string;
    extPath?: string;
    type?: Array<InstrumentTypes> | InstrumentTypes;
    settings: KfConfigItem[];
  }

  export type KfExtConfigs = Record<string, Record<string, KfExtConfig>>;

  export type KfUIExtConfigs = Record<
    string,
    {
      name: string;
      extPath: string;
      position: KfUIExtLocatorTypes;
      components:
        | {
            index: string;
          }
        | {
            entry: string;
            page: string;
          };
      daemon: Record<string, string>;
    }
  >;

  export interface SetKfConfigPayload {
    type: KungfuApi.ModalChangeType;
    title: string;
    config: KfExtConfig;
    initValue?: Record<string, KfConfigValue>;
  }

  export type ModalChangeType = 'add' | 'update';

  export interface MakeOrderInput {
    instrument_id: string;
    instrument_type: InstrumentTypeEnum;
    exchange_id: string;
    limit_price: number;
    volume: number;
    price_type: PriceTypeEnum;
    side: SideEnum;
    offset: OffsetEnum;
    hedge_flag: HedgeFlagEnum;
    parent_id: bigint;
  }

  export interface KfLogData {
    id: number;
    message: string;
    messageOrigin: string;
    messageForSearch: string;
  }

  export class KfNumList<T> {
    list: T[];
    limit: number;
    insert(item: T): void;
  }

  export interface ConfigStore {
    getAllConfig(): Record<string, KfConfigOrigin>;
    setConfig(
      category: string,
      group: string,
      name: string,
      mode: string,
      configValue: string,
    ): void;
    removeConfig(
      category: string,
      group: string,
      name: string,
      mode: string,
    ): void;
    getConfig(
      category: string,
      group: string,
      name: string,
      mode: string,
    ): KungfuApi.KfConfig;
  }

  export interface DataTable<T> {
    [hashed: string]: T;
    filter(key: string, value: string | number | bigint): DataTable<T>;
    nofilter(key: string, value: string | number | bigint): DataTable<T>;
    sort(key: string): T[];
    list(): T[];
  }

  export interface Asset {
    update_time: bigint; //更新时间
    trading_day: string; //交易日

    holder_uid: number;
    ledger_category: LedgerCategoryEnum;

    source_id: string; //柜台ID
    broker_id: string; // Broker ID
    account_id: string; //账号ID
    client_id: string; // client ID

    initial_equity: number; //期初权益
    static_equity: number; //静态权益
    dynamic_equity: number; //动态权益
    realized_pnl: number; //累计收益
    unrealized_pnl: number;
    avail: number; //可用资金
    market_value: number; //市值(股票)
    margin: number; //保证金(期货)
    accumulated_fee: number; //累计手续费
    intraday_fee: number; //当日手续费
    frozen_cash: number; //冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
    frozen_margin: number; //冻结保证金(期货)
    frozen_fee: number; //冻结手续费(期货)
    position_pnl: number; //持仓盈亏(期货)
    close_pnl: number; //平仓盈亏(期货)
  }

  export interface AssetSnapshot {
    update_time: bigint; //更新时间
    trading_day: string; //交易日

    holder_uid: number;
    ledger_category: LedgerCategoryEnum;

    source_id: string; //柜台ID
    broker_id: string; // Broker ID
    account_id: string; //账号ID
    client_id: string; // client ID

    initial_equity: number; //期初权益
    static_equity: number; //静态权益
    dynamic_equity: number; //动态权益
    realized_pnl: number; //累计收益
    unrealized_pnl: number;
    avail: number; //可用资金
    market_value: number; //市值(股票)
    margin: number; //保证金(期货)
    accumulated_fee: number; //累计手续费
    intraday_fee: number; //当日手续费
    frozen_cash: number; //冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
    frozen_margin: number; //冻结保证金(期货)
    frozen_fee: number; //冻结手续费(期货)
    position_pnl: number; //持仓盈亏(期货)
    close_pnl: number; //平仓盈亏(期货)
  }

  export interface Bar {
    trading_day: string; //交易日
    instrument_id: string; //合约代码
    exchange_id: string; //交易所代码

    start_time: bigint; //开始时间
    end_time: bigint; //结束时间

    open; //开
    close; //收
    low; //低
    high; //高

    volume: bigint; //区间交易量
    start_volume: bigint; //初始总交易量

    tick_count: number; //区间有效tick数
  }

  export interface DailyAsset {
    update_time: bigint; //更新时间
    trading_day: string; //交易日

    holder_uid: number;
    ledger_category: LedgerCategoryEnum;

    source_id: string; //柜台ID
    broker_id: string; // Broker ID
    account_id: string; //账号ID
    client_id: string; // client ID

    initial_equity: number; //期初权益
    static_equity: number; //静态权益
    dynamic_equity: number; //动态权益
    realized_pnl: number; //累计收益
    unrealized_pnl: number;
    avail: number; //可用资金
    market_value: number; //市值(股票)
    margin: number; //保证金(期货)
    accumulated_fee: number; //累计手续费
    intraday_fee: number; //当日手续费
    frozen_cash: number; //冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
    frozen_margin: number; //冻结保证金(期货)
    frozen_fee: number; //冻结手续费(期货)
    position_pnl: number; //持仓盈亏(期货)
    close_pnl: number; //平仓盈亏(期货)
  }

  export interface Instrument {
    instrument_id: string; //合约ID
    exchange_id: string; //交易所ID
    instrument_type: InstrumentTypeEnum; //合约类型

    product_id: number[]; //产品ID commit by JC

    contract_multiplier: number; //合约乘数
    price_tick: number; //最小变动价位

    open_date: string; //上市日
    create_date: string; //创建日
    expire_date: string; //到期日

    delivery_year: number; //交割年份
    delivery_month: number; //交割月

    is_trading: boolean; //当前是否交易

    long_margin_ratio: number; //多头保证金率
    short_margin_ratio: number; //空头保证金率

    uid_key: string;
    ukey: string;
  }

  export interface Order {
    parent_id: bigint; //母订单ID
    order_id: bigint; //订单ID
    insert_time: bigint; //订单写入时间
    update_time: bigint; //订单更新时间

    trading_day: string; //交易日
    instrument_id: string; //合约ID
    exchange_id: string; //交易所ID
    source_id: string; //柜台ID
    account_id: string; //账号ID
    client_id: string; // Client ID

    instrument_type: InstrumentTypeEnum; //合约类型

    limit_price: number; //价格
    frozen_price: number; //冻结价格, 市价单冻结价格为0

    volume: bigint; //数量
    volume_traded: bigint; //成交数量
    volume_left: bigint; //剩余数量

    tax: number; //税
    commission: number; //手续费

    status: OrderStatusEnum; //订单状态

    error_id: number; //错误ID
    error_msg: string; //错误信息

    side: SideEnum; //买卖方向
    offset: OffsetEnum; //开平方向
    hedge_flag: HedgeFlagEnum; //投机套保标识
    price_type: PriceTypeEnum; //价格类型
    volume_condition: VolumeConditionEnum; //成交量类型
    time_condition: TimeConditionEnum; //成交时间类型

    source: number;
    dest: number;
    uid_key: string;
  }

  export interface OrderResolved extends Order {
    source_resolved_data: KungfuApi.KfTradeValueCommonData;
    dest_resolved_data: KungfuApi.KfTradeValueCommonData;
    source_uname: string;
    dest_uname: string;
    update_time_resolved: string;
    latency_system: string;
    latency_network: string;
  }

  export interface OrderInput {
    order_id: bigint; //订单ID

    instrument_id: string; //合约代码
    exchange_id: string; //交易所代码
    source_id: string; //柜台ID
    account_id: string; //账号ID

    instrument_type: InstrumentTypeEnum; //合约类型

    limit_price: number; //价格
    frozen_price: number; //冻结价格

    volume: bigint; //数量

    side: SideEnum; //买卖方向
    offset: OffsetEnum; //开平方向
    hedge_flag: HedgeFlagEnum; //投机套保标识
    price_type: PriceTypeEnum; //价格类型
    volume_condition: VolumeConditionEnum; //成交量类型
    time_condition: TimeConditionEnum; //成交时间类型

    parent_id: bigint; //母订单ID
    insert_time: bigint; //写入时间

    source: number;
    dest: number;
    uid_key: string;
  }

  export interface OrderStat {
    order_id: bigint;
    md_time: bigint;
    input_time: bigint;
    insert_time: bigint;
    ack_time: bigint;
    trade_time: bigint;

    source: number;
    dest: number;
    uid_key: string;
  }

  export interface OrderAction {
    order_id: bigint;
    order_action_id: bigint;
    action_flag: OrderActionFlagEnum;
    price: number;
    volume: number;
    insert_time: bigint;
  }

  export interface Position {
    update_time: bigint; //更新时间
    trading_day: string; //交易日

    instrument_id: string; //合约ID
    exchange_id: string; //交易所ID

    instrument_type: InstrumentTypeEnum; //合约类型

    holder_uid: number;
    ledger_category: LedgerCategoryEnum;

    source_id: string; //柜台ID
    account_id: string; //账号ID
    client_id: string; // Client ID

    direction: DirectionEnum; //持仓方向

    volume: bigint; //数量
    yesterday_volume: bigint; //昨仓数量
    frozen_total: bigint; //冻结数量
    frozen_yesterday: bigint; //冻结昨仓

    last_price: number; //最新价
    avg_open_price: number; //开仓均价
    position_cost_price: number; //持仓成本价
    close_price: number; //收盘价(股票和债券)
    pre_close_price: number; //昨收价(股票和债券)
    settlement_price: number; //结算价(期货)
    pre_settlement_price: number; //昨结算(期货)
    margin: number; //保证金(期货)
    position_pnl: number; //持仓盈亏(期货)
    close_pnl: number; //平仓盈亏(期货)
    realized_pnl: number; //已实现盈亏
    unrealized_pnl: number; //未实现盈亏

    uid_key: string;
  }

  export interface PositionResolved extends Position {
    account_id_resolved: string;
    instrument_id_resolved: string;
  }

  export interface Quote {
    source_id: string; //柜台ID
    trading_day: string; //交易日

    data_time: bigint; //数据生成时间

    instrument_id: string; //合约ID
    exchange_id: string; //交易所ID

    instrument_type: InstrumentTypeEnum; //合约类型

    pre_close_price: number; //昨收价
    pre_settlement_price: number; //昨结价
    last_price: number; //最新价
    volume: bigint; //数量
    turnover: number; //成交金额
    pre_open_interest: number; //昨持仓量
    open_interest: number; //持仓量
    open_price: number; //今开盘
    high_price: number; //最高价
    low_price: number; //最低价
    upper_limit_price: number; //涨停板价
    lower_limit_price: number; //跌停板价
    close_price: number; //收盘价
    settlement_price: number; //结算价

    bid_price: number[]; //申买价
    ask_price: number[]; //申卖价
    bid_volume: bigint[]; //申买量
    ask_volume: bigint[]; //申卖量
  }

  export interface TimeValue {
    update_time: bigint;
    tag_a: string;
    tag_b: string;
    tag_c: string;
    value: string;
  }

  export interface Trade {
    trade_id: bigint; //成交ID
    order_id: bigint; //订单ID
    parent_order_id: bigint; //母订单ID
    trade_time: bigint; //成交时间

    trading_day: string; //交易日

    instrument_id: string; //合约ID
    exchange_id: string; //交易所ID
    source_id: string; //柜台ID
    account_id: string; //账号ID
    client_id: string; // Client ID

    instrument_type: InstrumentTypeEnum; //合约类型

    side: SideEnum; //买卖方向
    offset: OffsetEnum; //开平方向
    hedge_flag: HedgeFlagEnum; //投机套保标识

    price: number; //成交价格
    volume: bigint; //成交量
    close_today_volume: bigint; //平今日仓量(期货)

    tax: number; //税
    commission: number; //手续费

    source: number;
    dest: number;
    uid_key: string;
  }

  export interface TradeResolved extends Trade {
    source_resolved_data: KungfuApi.KfTradeValueCommonData;
    dest_resolved_data: KungfuApi.KfTradeValueCommonData;
    source_uname: string;
    dest_uname: string;
    trade_time_resolved: string;
    kf_time_resovlved: string;
    latency_trade: string;
  }

  export interface TradingData {
    Asset: DataTable<Asset>;
    AssetSnapshot: DataTable<AssetSnapshot>;
    Bar: DataTable<Bar>;
    DailyAsset: DataTable<DailyAsset>;
    Instrument: DataTable<Instrument>;
    Order: DataTable<Order>;
    OrderInput: DataTable<OrderInput>;
    OrderStat: DataTable<OrderStat>;
    Position: DataTable<Position>;
    Quote: DataTable<Quote>;
    TimeValue: DataTable<TimeValue>;
    Trade: DataTable<Trade>;
  }

  export interface TradingDataNameToType {
    Asset: Asset;
    AssetSnapshot: AssetSnapshot;
    Bar: Bar;
    DailyAsset: DailyAsset;
    Instrument: Instrument;
    Order: Order;
    OrderInput: OrderInput;
    OrderStat: OrderStat;
    Position: Position;
    Quote: Quote;
    TimeValue: TimeValue;
    Trade: Trade;
  }

  export type TradingDataTypes =
    | Asset
    | AssetSnapshot
    | Bar
    | DailyAsset
    | Instrument
    | Order
    | OrderInput
    | OrderStat
    | Position
    | Quote
    | TimeValue
    | Trade;

  export type TradingDataTypeName = keyof TradingData;

  export interface Commission {
    product_id: string;
    exchange_id: string;
    instrument_type: InstrumentTypeEnum;
    mode: CommissionModeEnum;
    open_ratio: number;
    close_ratio: number;
    close_today_ratio: number;
    min_commission: number;
  }

  export interface Watcher {
    appStates: Record<string, BrokerStateStatusEnum>;
    ledger: TradingData;
    state: TradingData;

    isLive(): boolean;
    isStarted(): boolean;
    isUsable(): boolean;
    createTask(): void;
    sync(): void;
    isReadyToInteract(kfLocation: KfLocation | KfConfig): boolean;
    getLocationUID(kfLocation: KfLocation | KfConfig): string;
    getLocation(hashedKey: string | number): KfLocation;
    requestMarketData(
      kfLocation: KfLocation,
      exchangeId: string,
      instrumentId: string,
    ): void;
    cancelOrder(
      orderAction: OrderAction,
      tdLocation: KfLocation,
      strategyLocation?: KfLocation,
    ): void;
    issueOrder(
      orderInput: OrderInput,
      tdLocation: KfLocation,
      strategyLocation?: KfLocation,
    ): void;
    now(): bigint;
  }

  export interface HistoryStore {
    selectPeriod(from: string, to: string): TradingData;
  }

  export interface CommissionStore {
    getAllCommission(): Commission[];
    setAllCommission(commissions: Commission[]): boolean;
  }

  export interface Longfist {
    Asset(): Asset;
    AssetSnapshot(): AssetSnapshot;
    Bar(): Bar;
    DailyAsset(): DailyAsset;
    Instrument(): Instrument;
    Order(): Order;
    OrderInput(): OrderInput;
    OrderAction(): OrderAction;
    OrderStat(): OrderStat;
    Position(): Position;
    Quote(): Quote;
    TimeValue(): TimeValue;
    Trade(): Trade;
    Commission(): Commission;
  }

  export interface Kungfu {
    ConfigStore(kfHome: string): ConfigStore;
    CommissionStore(kfHome: string): CommissionStore;
    History(kfHome: string): HistoryStore;
    longfist: Longfist;
    watcher(
      kfHome: string,
      hashedId: string,
      bypassQuote?: boolean,
      bypassRestore?: boolean,
    ): Watcher | null;
    shutdown(): void;
    formatStringToHashHex(id: string): string;
    formatTime(nano: bigint, format: string): string;
    hash(str: string): string;
  }

  export interface InstrumentForSub {
    uidKey: string;
    exchangeId: string;
    instrumentId: string;
    instrumentType: InstrumentTypeEnum;
    mdLocation: KfLocation;
  }

  export interface InstrumentResolved {
    instrumentId: string;
    instrumentType: InstrumentTypeEnum;
    instrumentName: string;
    exchangeId: string;
    id: string;
    ukey: string;
  }

  export interface KfLocationBase {
    group: string;
    name: string;
  }

  export interface KfLocation extends KfLocationBase {
    category: KfCategoryTypes;
    mode: KfModeTypes;
  }

  export interface KfLocationOrigin extends KfLocationBase {
    category: KfCategoryEnum;
    mode: KfModeEnum;
  }

  export interface KfConfigOrigin extends KfLocationOrigin {
    location_uid: number;
    value: string;
  }

  export interface KfConfig extends KfLocation {
    location_uid: number;
    value: string;
  }

  export interface KfExtraLocation extends KfLocation {
    category: string;
    group: string;
    name: string;
    [prop: string]: any;
  }

  export interface KfDaemonLocation extends KfExtraLocation {
    cwd: string;
    script: string;
  }

  export type ScheduleTaskMode = 'restart' | 'start' | 'stop';

  export interface ScheduleTask {
    hour: string;
    minute: string;
    second: string;
    mode: ScheduleTaskMode;
    processId: string;
  }
}

declare module '@kungfu-trader/kungfu-core' {
  export function kungfu(): KungfuApi.Kungfu;
}

declare namespace Code {
  import { Stats } from 'fs-extra';
  import { SpaceTabSettingEnum, SpaceSizeSettingEnum } from './enums';
  export interface Strategy {
    strategy_id: string;
    strategy_path: string;
    add_time: number;
  }

  export interface FileProps {
    root?: boolean;
    filePath?: string;
    ext?: string;
    isDir?: boolean;
  }

  export interface FileIds {
    file: number[];
    folder: number[];
  }
  export interface FileTreeByPath {
    ids: FileIds;
    fileTree: IFileTree;
  }
  export interface FileData {
    id: number;
    parentId: number;
    isDir: boolean;
    name: string;
    ext: string;
    filePath: string;
    children: FileIds;
    stats: Stats;
    root: boolean;
    open: boolean;
    fileId?: number;
    isEntryFile?: boolean;
  }

  export type IFileTree = Record<string, FileData>;

  export interface IFileNode {
    filePath: string;
    root?: string;
    children?: IFileNode;
    name: string;
  }

  export interface ICodeSetting {
    tabSpaceType: SpaceTabSettingEnum;
    tabSpaceSize: SpaceSizeSettingEnum;
  }
}
