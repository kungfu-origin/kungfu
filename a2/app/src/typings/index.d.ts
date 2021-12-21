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

interface MdRow {
    sourceId: string;
    stateStatus: ProcessStatusTypes;
    processStatus: boolean;
}

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

type KfBusEvent = ResizeEvent | ProcessStatusChangeEvent | MainProcessEvent;

interface KfTradingDataTableHeaderConfig {
    name: string;
    dataIndex: string;
    width?: number;
    flex?: number;
    type:
        | 'number'
        | 'string'
        | 'nanoTime'
        | 'exchange'
        | 'offset'
        | 'side'
        | 'priceType'
        | 'direction';
}
