declare const __resources: string;
declare const __python_version: string;
declare const __git_commit_version: string;
declare const __build_timestamp: number;

declare module NodeJS {
    interface Global {
        __resources: string;
        __kfResourcesPath: string;
    }
}

interface ConfigStore {
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
}

interface DataTable<T> extends T {
    [prop: string]: T;
    filter(key: string, value: string | number | bigint): DataTable<T>;
    nofilter(key: string, value: string | number | bigint): DataTable<T>;
    sort(key: string): T[];
    list(): T[];
}

interface Asset {
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
    close_pn: number; //平仓盈亏(期货)
}

interface AssetSnapshot {
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

interface Bar {
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

interface DailyAsset {
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

interface Instrument {
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
}

interface Order {
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
    frozen_price: number; //冻结价格，市价单冻结价格为0

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
}

interface OrderInput {
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
}

interface OrderStat {
    order_id: bigint;
    md_time: bigint;
    input_time: bigint;
    insert_time: bigint;
    ack_time: bigint;
    trade_time: bigint;

    source: number;
    dest: number;
}

interface Position {
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
}

interface Quote {
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

interface TimeValue {
    update_time: bigint;
    tag_a: string;
    tag_b: string;
    tag_c: string;
    value: string;
}

interface Trade {
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
}

interface TradingData {
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

type TradingDataTypes =
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
    | Trade
    | Trade;

interface Watcher {
    appStates: Record<string, BrokerStateStatusEnum>;
    ledger: TradingData;
    state: TradingData;

    isLive(): boolean;
    isStarted(): boolean;
    isUsable(): boolean;
    setup(): boolean;
    step(): boolean;
    updateQuote(): void;
    isReadyToInteract(kfLocation: KfLocation | KfConfig): boolean;
    getLocationUID(kfLocation: KfLocation | KfConfig): string;
    getLocation(hashedKey: string | number): KfLocation;
}

declare module '@kungfu-trader/kungfu-core' {
    export function kungfu(): {
        longfist: any;
        ConfigStore(kfHome: string): ConfigStore;
        watcher(
            kfHome: string,
            hashedId: string,
            bypassQuote = false,
            bypassRestore = false,
        ): Watcher | null;
        shutdown(): void;
        formatStringToHashHex(id: string): string;
        formatTime(nano: bigint, format: string): string;
    };
}

interface KfLogData {
    id: number;
    message: string;
    messageOrigin: string;
    messageForSearch: string;
}

class KfNumList<T> {
    list: T[];
    limit: number;
    insert(item: T): void;
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

declare type ModalChangeType = 'add' | 'update';
