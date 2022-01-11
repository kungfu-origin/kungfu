namespace KfLayout {
    type ContentId = string;
    type BoardId = number;

    interface BoardInfo {
        paId: number;
        children?: number[];
        direction: KfLayoutDirection;
        contents?: ContentId[];
        current?: ContentId;
        width?: number | string;
        height?: number | string;
    }

    interface BoardsMap {
        [prop: BoardId]: BoardInfo;
    }

    interface ContentData {
        contentId: ContentId;
        boardId: BoardId;
    }
}
interface AntTableColumn {
    title: string;
    dataIndex: string;
    key?: string;
    width?: number | string;
    sorter?: boolean | { compare: (a: any, b: any) => number };
    align?: string;
    fixed?: string;
}

type AntTableColumns = Array<AntTableColumn>;

interface ResizeEvent {
    tag: 'resize';
}

interface ProcessStatusChangeEvent {
    tag: 'processStatus';
    name: string;
    status: Pm2ProcessStatusTypes;
}

interface MainProcessEvent {
    tag: 'main';
    name: string;
}

interface TradingDataUpdateEvent {
    tag: 'tradingDataUpdate';
    tradingDataType: TradingDataTypeName;
}

interface ExportTradingDataEvent {
    tag: 'export';
    tradingDataType: TradingDataTypeName | 'all';
    currentKfLocation?: KfLocation | KfConfig | undefined;
}

interface TriggeOrderBook {
    tag: 'orderbook';
    instrument: InstrumentResolved;
}

interface ExtraOrderInput {
    side: SideEnum;
    offset?: OffsetEnum;
    volume: number | bigint;
    price: number;
    accountId?: string;
}

interface TiggerOrderBookUpdate {
    tag: 'orderBookUpdate';
    orderInput: InstrumentResolved | ExtraOrderInput;
}

interface TriggerMakeOrder {
    tag: 'makeOrder';
    orderInput: InstrumentResolved | ExtraOrderInput;
}

type KfBusEvent =
    | ResizeEvent
    | ProcessStatusChangeEvent
    | MainProcessEvent
    | TradingDataUpdateEvent
    | TriggeOrderBook
    | TiggerOrderBookUpdate
    | TriggerMakeOrder
    | ExportTradingDataEvent;

type TradingDataItem = KungfuApi.Position | KungfuApi.Order | KungfuApi.Trade;
interface KfTradingDataTableHeaderConfig {
    name: string;
    dataIndex: string;
    width?: number;
    flex?: number;
    type?:
        | 'number'
        | 'string'
        | 'source'
        | 'nanoTime'
        | 'exchange'
        | 'offset'
        | 'side'
        | 'priceType'
        | 'direction'
        | 'actions';
}

declare module 'worker-loader!*' {
    class WebpackWorker extends Worker {
        constructor();
    }

    export = WebpackWorker;
}
