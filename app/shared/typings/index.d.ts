declare var __resources: string;

declare module NodeJS {
    interface Global {
        __resources: string
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

interface PosData {
    id: string;
    instrumentId: string;
    direction: string;
    yesterdayVolume: string;
    todayVolume: string;
    avgPrice: string;
    lastPrice: string;
    unRealizedPnl: string;
}

interface PosInputData {
    instrument_id: string;
    direction: number;
    yesterday_volume: string;
    volume: string;
    last_price: string;
    margin: string;
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
    tip?: string
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