
declare var __resources: string;


interface SourceAccountId {
    source: string,
    id: string
}

interface String {
    toAccountId(): string;
    parseSourceAccountId(): SourceAccountId;
}

interface NumberToStringObject {
    [propName: number]: string;
}
interface StringToStringObject {
    [propName: string]: string;
}

interface LogLineData {
    message: string;
    type: string;
    timestamp: string;
    [propName: string]: any;
}

interface LogMessageData {
    updateTime: string;
    type: string;
    pid: string;
    action: string;
    message: string;
}

interface OrderData {
    id: string;
    insertTime: string;
    instrumentId: string;
    side: string;
    offset: string;
    limitPrice: number;
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
    yesterdayVolume: number;
    todayVolume: number;
    totalVolume: number;
    costPrice: number | string;
    lastPrice: number | string;
    unRealizedPnl: number | string;
}

interface TradeData {
    id: string;
    tradeTime: string;
    instrumentId: string;
    side: string;
    offset: string;
    price: number;
    volume: number;
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
    price: number;
    volume: number;
    [propName: string]: any;
}