interface ProcPayload {
    topic: string;
    data: {
        quotes: QuoteData[];
        positions: PosData[];
        orders: OrderData[];
        instruments: InstrumentInputData[];
        avgVolume: StringToNumberObject;
    }
}


interface StringToQuoteData {
    [prop: string]: QuoteData
}


interface BackWardTraderPipeData {
    seconds: number;
    quotes: StringToQuoteData;
    instruments: StringToInstrumentData;
    avgVolume: StringToNumberObject;
    orders: OrderData[];
    pos: StringToPosData | null;
}

interface StringToInstrumentData {
    [prop: string]: InstrumentInputData
}
