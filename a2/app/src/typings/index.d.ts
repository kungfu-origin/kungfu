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
    sorter?: boolean;
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

interface TriggerOrderBook {
    tag: 'orderbook';
    instrument: InstrumentResolved;
}

type KfBusEvent =
    | ResizeEvent
    | ProcessStatusChangeEvent
    | MainProcessEvent
    | TradingDataUpdateEvent
    | TriggerOrderBook
    | ExportTradingDataEvent;

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