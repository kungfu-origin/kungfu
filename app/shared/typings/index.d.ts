declare var __resources: string;

declare module NodeJS {
    interface Global {
        __resources: string
    }
     
    interface Process {
        resourcesPath: string
    }

}



declare module "*.json" {
    const value: any;
    export default value;
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

interface Account {
    account_id: string;
    source_name: string;
    receive_md: boolean;
    config?: any;
    [propName: string]: any;
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
    instrumentId: string;
    side: string;
    offset: string;
    limitPrice: string;
    volumeTraded: string;
    statusName: string;
    status: string;
    clientId: string;
    accountId: string;
    orderId: string;
    exchangeId: string;
}

interface OrderInputData {
    update_time: number,
    insert_time: number,
    order_id: string,
    instrument_id: string,
    side: number,
    offset: number,
    volume: number,
    volume_traded: number,
    status: any,
    limit_price: number,
    client_id: string,
    account_id: string,
    exchange_id: string
}

interface PosData {
    id: string;
    instrumentId: string;
    direction: string;
    yesterdayVolume: string;
    todayVolume: string;
    totalVolume: string;
    avgPrice: string;
    lastPrice: string;
    unRealizedPnl: string;
}

interface PosInputData {
    instrument_id: string;
    direction: number;
    yesterday_volume: number;
    volume: number;
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
    volume: string;
    clientId: string;
    accountId: string;
}

interface TradeInputData {
    account_id: string;
    client_id: string;
    trade_id: string;
    instrument_id: string;
    trade_time: number;
    offset: number;
    side: number;
    price: string;
    volume: string;
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
    config: AccountSettingItem[]
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


interface NumList {
    list: any[];
    len: number;
    num: number;
    insert: Function;
}
