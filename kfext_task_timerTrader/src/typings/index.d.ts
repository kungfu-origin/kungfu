

interface ProcPayload {
    topic: string;
    data: {
        quotes: QuoteData[];
        positions: PosData[];
        orders: OrderData[];
    }
}

interface OrderIdData {
    status: number;
    orderId: string
}

interface VolumeRecordData {
    d: number;
    v: number;
}

interface direcData {
    d: number; //方向
    n: string;
    dc: number; //反方向
}

interface TimeCountData {
    count: number;
    second: number;
}

interface TimeTraderPipData {
    timeCountData: TimeCountData;
    quotes: StringToQuoteData;
    positions: StringToPosData | null;
    orders: OrderData[];
    [propName: string]: any;
}

interface TradeTarget  {
    offset: number,
    volume: number
}