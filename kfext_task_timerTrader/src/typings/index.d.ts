

interface ProcPayload {
    topic: string;
    data: {
        quotes: QuoteData[];
        positions: PosData[];
    }
}
