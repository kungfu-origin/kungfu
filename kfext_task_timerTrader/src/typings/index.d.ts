

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