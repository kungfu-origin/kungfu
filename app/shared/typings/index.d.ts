declare var __resources: string;

declare var python_version: string;

declare module NodeJS {
    interface Window { 
        fileId: number;
    }

    interface Global {
        __resources: string;
        __watcher: any;
        __locator: any;
    }
     
    interface Process {
        resourcesPath: string
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

interface StringToStringObject {
    [propName: string]: string;
}

interface StringToNumberObject {
    [propName: string]: number;
}

interface StringToFunctionObject {
    [propName: string]: Function;
}

interface StringToAny {
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
    }
}

interface StringToProcessStatusDetail {
    [propName: string]: ProcessStatusDetail
}

interface SourceType {
    [propName: string]: {
        name: string,
        kfId: number,
        color: string
    }
}

interface Account {
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
    type: string,
    updateTime: string,
    message: string,
}

interface LogDataOrigin {
    timestamp: string,
    type: string,
    app_name: string,
    message: string,
    process_id: number
}


interface OrderData {
    id: string;
    updateTime: string,
    updateTimeNum: number;
    orderId: string;
    instrumentId: string;
    side: string;
    offset: string;
    limitPrice: string;
    volumeTraded: string;
    statusName: string;
    status: string;
    clientId: string;
    accountId: string;
    sourceId: string,
    exchangeId: string;
}

interface OrderInputData {
    update_time: bigint,
    insert_time: bigint,
    order_id: bigint,
    instrument_id: string,
    side: number,
    offset: number,
    volume: bigint,
    volume_left: bigint,
    volume_traded: bigint,
    status: any,
    limit_price: number,
    client_id: string,
    account_id: string,
    source_id: string,
    exchange_id: string
}

interface PosData {
    id: string;
    instrumentId: string;
    direction: string;
    yesterdayVolume: Number;
    todayVolume: Number;
    totalVolume: Number;
    avgPrice: string;
    lastPrice: string;
    unRealizedPnl: string;
}

interface PosInputData {
    instrument_id: string;
    direction: number;
    yesterday_volume: bigint;
    volume: bigint;
    last_price: number;
    margin: number;
    [propName: string]: any;
}

interface TradeData {
    id: string;
    instrumentId: string;
    updateTime: string;
    updateTimeNum: number;
    side: string;
    offset: string;
    price: string;
    volume: Number;
    clientId: string;
    accountId: string;
    sourceId: string;
}

interface TradeInputData {
    account_id: string;
    client_id: string;
    source_id: string;
    trade_id: bigint;
    instrument_id: string;
    trade_time: bigint;
    offset: number;
    side: number;
    price: string;
    volume: bigint;
    [propName: string]: any;
}

interface AssetInputData {
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
    margin: number,
    ledger_category: number;
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

interface TradingDataFilter {
    instrumentId?: string;
    type?: string;
    id?: string;
    dateRange?: string[];
}

interface AccountSettingItem {
    key: string,
    name: string,
    type: string,
    errMsg?: string,
    required?: boolean,
    validator?: any[],
    tip?: string,
    data?: any[],
    default?: any
}

interface AccountSettingOrigin {
    name: string,
    type: string,
    key: string,
    td_config: AccountSettingItem[],
    md_config: AccountSettingItem[],
}

interface AccountSetting {
    name: string,
    source: string,
    type: string,
    typeName: string,
    key: string,
    config: AccountSettingItem[],
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
    category: number;
    group: string;
    name: string;
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
    category: string,
    group: string,
    name: string,
    mode: string
}