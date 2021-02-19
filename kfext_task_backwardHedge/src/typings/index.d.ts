interface ProcPayload {
    topic: string;
    data: {
        quotes: QuoteData[];
        positions: PosData[];
        orders: OrderData[];
        instruments: InstrumentInputData[];
    }
}


interface StringToQuoteData {
    [prop: string]: QuoteData
}


interface BackWardTraderPipeData {
    seconds: number;
    quotes: StringToQuoteData;
    instruments: StringToInstrumentData
}

interface StringToInstrumentData {
    [prop: string]: InstrumentInputData
}