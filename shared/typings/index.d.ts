declare var __resources: string;

declare var python_version: string;

declare module NodeJS {
    interface Window { 
        fileId: number;
        ELEC_WIN_MAP: any;
        pm2: any;
    }

    interface Global {
        __resources: string;
        __watcher: any;
        __locator: any;
    }
     
    interface Process {
        resourcesPath: string,
    }
}

declare module "*.json" {
    const value: any;
    export default value;
}

interface SqlTable {
    table: string;
    sql: string;
}

interface NumberToStringObject {
    [propName: number]: string;
}

interface NumberToAnyObject {
    [propName: number]: any;
}


interface StringToStringObject {
    [propName: string]: string;
}

interface StringToNumberObject {
    [propName: string]: number;
}

interface StringToFunctionObject {
    [propName: string]: Function;
}

interface StringToAnyObject {
    [propName: string]: any
}

interface NormalObject {
    [propName: string]: any;
}

interface ProcessStatusDetail {
    status: string;
    monit: {
        memory: bigint;
        cpu: number;
    },
    pid: string,
    name: string,
    created_at: string,
    script: string,
    cwd: string,
    args: string
}

interface StringToProcessStatusDetail {
    [propName: string]: ProcessStatusDetail
}

interface SourceType {
    [propName: string]: {
        name: string;
        kfId: number;
        color: string
    }
}

interface Td {
    account_id: string;
    source_name: string;
    config?: any;
    [propName: string]: any;
}

interface Md {
    source_name: string;
    config: any
}

interface Strategy {
    strategy_id: string;
    strategy_path: string;
    add_time: number;
}

interface MessageData {
    type: string;
    message: string;
}

interface LogData {
    type: string;
    updateTime: string;
    message: string;
}

interface LogDataOrigin {
    timestamp: string;
    type: string;
    app_name: string;
    message: string;
    process_id: number
}

interface OrderInputData {
    id: string;

    updateTime: string;
    updateTimeMMDD: string;
    updateTimeNum: number;
    orderId: string;
    parentId: string;

    instrumentId: string;
    instrumentType: string;
    instrumentTypeOrigin: number;

    exchangeId: string;
    sourceId: string;
    accountId: string;

    limitPrice: string;
    frozenPrice: string;
    volume: string;

    side: string;
    sideOrigin: number;
    offset: string;
    offsetOrigin: number;
    hedgeFlag: string;
    hedgeFlagOrigin: number;
    priceType: string;
    priceTypeOrigin: number;

    source: string;
    dest: string;
    
}

interface OrderInputOriginData {
    order_id: bigint;
    parent_id: bigint;
    ts: bigint;

    instrument_id: string;
    exchange_id: string;
    source_id: string;
    account_id: string;
    instrument_type: number;
    limit_price: number;
    frozen_price: number;
    volume: bigInt;
    
    side: number;
    offset: number;
    hedge_flag: number;
    price_type: number;

    source: string;
    dest: string;
}


interface OrderData {
    id: string;
    
    updateTime: string;
    updateTimeMMDD: string;
    updateTimeNum: number;
    orderId: string;
    parentId: string;

    instrumentId: string;
    instrumentType: string;
    instrumentTypeOrigin: number;
    exchangeId: string;

    side: string;
    sideOrigin: number;
    offset: string;
    offsetOrigin: number;
    hedgeFlag: string;
    hedgeFlagOrigin: number;
    priceType: string;
    priceTypeOrigin: number;
    volumeCondition: string;
    timeCondition: string;

    limitPrice: string;
    frozenPrice: string;

    volume: string;
    volumeLeft: string;
    volumeTraded: string;

    statusName: string;
    status: string;

    tax: number;
    comission: number;

    errorId: number;
    errorMsg: string;

    clientId: string;
    accountId: string;
    sourceId: string;

    source: string;
    dest: string;
    
    latencySystem?: string | number;
    latencyNetwork?: string | number;
}

interface OrderOriginData {    
    parent_id: bigint;
    order_id: bigint;
    ts: bigint;

    instrument_id: string;
    instrument_type: number;
    exchange_id: string;

    side: number;
    offset: number;
    hedge_flag: number;
    price_type: number;
    volume_condition: number;
    time_condition: number;
    
    volume: bigint;
    volume_left: bigint;
    volume_traded: bigint;
    
    status: any;
    
    limit_price: number;
    frozen_price: number;

    tax: number;
    commission: number;

    error_id: number;
    error_msg: string;


    client_id: string;
    account_id: string;
    source_id: string;

    source: string;
    dest: string;
}

interface TradeData {
    id: string;
    updateTime: string;
    updateTimeMMDD: string;
    updateTimeNum: number;
    orderId: string;
    parentOrderId: string;

    instrumentId: string;
    instrumentType: string;
    instrumentTypeOrigin: number;
    exchangeId: string;
    
    side: string;
    sideOrigin: number;
    offset: string;
    offsetOrigin: number;
    hedgeFlag: string;
    hedgeFlagOrigin: number;

    price: string;
    volume: number;

    clientId: string;
    accountId: string;
    sourceId: string;

    source: string;
    dest: string;
    
    tax: string;
    commission: string;
}

interface TradeOriginData {
    trade_id: bigint;
    order_id: bigint;
    parent_order_id: bigint;
    ts: bigint;
  
    instrument_id: string;
    instrument_type: number;
    exchange_id: string;

    offset: number;
    side: number;
    hedge_flag: number;
    
    price: string;
    volume: bigint;

    account_id: string;
    client_id: string;
    source_id: string;
    
    source: string;
    dest: string;
    
    tax: number;
    commission: number;
}

interface PosData {
    id: string;
    instrumentId: string;
    instrumentType: number;
    direction: string;
    directionOrigin: number;
    yesterdayVolume: number;
    todayVolume: number;
    totalVolume: number;
    avgPrice: string;
    lastPrice: string;
    totalPrice: string;
    totalMarketPrice: string;
    unRealizedPnl: string;
    exchangeId: string;
    accountId: string;
    sourceId: string;
    clientId: string;
    accountIdResolved: string;
}

interface PosOriginData {
    instrument_id: string;
    instrument_type: number;
    direction: number;
    yesterday_volume: bigint;
    unrealized_pnl: number,
    volume: bigint;
    last_price: number;
    margin: number;
    exchange_id: string;
    account_id: string;
    source_id: string;
    client_id: string;
    [propName: string]: any;
}

interface AssetData {
    accountId: string;
    clientId: string;
    initialEquity: string;
    staticEquity: string;
    dynamicEquity: string;
    realizedPnl: string;
    unRealizedPnl: string;
    avail: string;
    marketValue: string;
    margin: string;
}

interface AssetOriginData {
    account_id: string;
    source_id: string;
    client_id: string;
    initial_equity: number;
    static_equity: number;
    dynamic_equity: number;
    realized_pnl: number;
    unrealized_pnl: number;
    avail: number;
    market_value: number;
    margin: number;
    ledger_category: number;
}

interface OrderStatData {
    ackTime: number;
    insertTime: number;
    mdTime: number;
    orderId: string;
    dest: string;
    source: string;
    latencySystem: string;
    latencyNetwork: string;
    latencyTrade: string;
    tradeTime: string;
    tradeTimeMMDD: string;
    tradeTimeNum: number;
}

interface OrderStatOriginData {
    ack_time: bigint;
    insert_time: bigint;
    md_time: bigint;
    trade_time: bigint;
    order_id: bigint;
    dest: string;
    source: string
}

interface AccountSettingItem {
    key: string;
    name: string;
    type: string;
    errMsg?: string;
    required?: boolean;
    validator?: any[];
    tip?: string;
    data?: any[];
    default?: any
}

interface AccountSettingOrigin {
    name: string;
    type: string;
    key: string;
    td_config: AccountSettingItem[];
    md_config: AccountSettingItem[];
}

interface AccountSetting {
    name: string;
    source: string;
    type: string;
    typeName: string;
    key: string;
    config: AccountSettingItem[];
}

interface Sources {
    [propName: string]: AccountSetting
}

interface StringToSource {
    [propName: string]: Sources
}


interface NumList {
    list: any[];
    len: number;
    num: number;
    insert: Function;
}


interface MdTdState {
    processId?: string;
    state: number
}

interface StringToMdTdState {
    [processName: string]: MdTdState
}



interface SystemConfigChildArgsItemData {
    key: string;
    value: string;
}

interface SystemConfigChildSelectItemData {
    value: string | number;
    name: string | number;
}

interface DataWithAccountIdClientId {
    client_id: string;
    account_id: string;
    source_id: string;
    [propName: string]: string | number;
}

interface StringToDataWithAccountIdClientIdData {
    [propName: string]: { 
        [propName: string]: DataWithAccountIdClientId[]
    }
}

interface KungfuLocation {
    category: string;
    group: string;
    name: string;
    mode: string
}

interface QuoteDataInput {
    close_price: number
    data_time: bigint
    exchange_id: string
    high_price: number
    instrument_id: string
    instrument_type: number
    last_price: number
    low_price: number
    lower_limit_price: number
    open_interest: number
    open_price: number
    pre_close_price: number
    pre_open_interest: number
    pre_settlement_price: number
    settlement_price: number
    source_id: string
    trading_day: string
    turnover: number
    upper_limit_price: number
    volume: bigint
    ask_price: Array<number>
    ask_volume: Array<BigInt>
    bid_price: Array<number>
    bid_volume: Array<BigInt>
}

interface QuoteData {
    id: string,
    closePrice: string
    dataTime: number
    dataTimeNumber: string,
    exchangeId: string
    highPrice: string
    instrumentId: string
    instrumentType: string
    instrumentTypeOrigin: number;
    lastPrice: string
    lowPrice: string
    lowerLimitPrice: string
    openInterest: number
    openPrice: string
    preClosePrice: string
    preOpenInterest: number
    preSettlementPrice: string
    settlementPrice: string
    sourceId: string
    tradingDay: string
    turnover: number
    upperLimitPrice: string
    volume: number,
    askPrices: Array<string>,
    askVolumes: Array<string>,
    bidPrices: Array<string>,
    bidVolumes: Array<string>,
}

interface InstrumentInputData {
    contract_multiplier: number;
    create_date: string;
    delivery_month: number;
    delivery_year: number;
    exchange_id: string;
    expire_date: string;
    instrument_id: string;
    instrument_type: number;
    is_trading: boolean;
    long_margin_ratio: number;
    open_date: string;
    price_tick: number;
    product_id: string[];
    short_margin_ratio: number;
}

interface Pm2Options {
    name: string;
    args: string;
    cwd?: string;
    script?: string;
    interpreter?: string;
    maxRestarts?: number;
    autorestart?: boolean;
}

interface TickerSet {
    name: string;
    config: TickerSet
}

interface TickerInTickerSet {
    source: string;
    exchangeId: string;
    instrumentId: string;
}

interface TickerSet {
    name: string;
    tickers: TickerInTickerSet[]
}

interface stringToTickerSet {
    [ propName: string ]: TickerSet;
}

interface StringToQuoteData {
    [prop: string]: QuoteData
}

interface StringToPosData {
    [prop: string]: PosData
}