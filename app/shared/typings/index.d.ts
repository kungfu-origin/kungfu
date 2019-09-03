declare var __resources: string;

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
}

interface Strategy {
    stratgy_id: string;
    strategy_path: string;
    add_time: number;
}

interface MessageData {
    type: string;
    message: string;
}

interface OrderData {
    id: string;
    insertTime: string;
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
    tradeTime: string;
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
