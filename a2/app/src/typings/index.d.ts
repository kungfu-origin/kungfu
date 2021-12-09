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

interface ResizeEvent {
    tag: 'resize';
}

type KfEvent = ResizeEvent;

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

interface TdRow {
    accountName: string;
    accountId: string;
    sourceId: string;
    stateStatus: ProcessStatusTypes;
    processStatus: boolean;
    realizedPnl: number;
    unrealizedPnl: number;
    marketValue: number;
    margin: number;
    avail: number;
}

interface MdRow {
    sourceId: string;
    stateStatus: ProcessStatusTypes;
    processStatus: boolean;
}

interface StrategyRow {
    strategyId: string;
    stateStatus: ProcessStatusTypes;
    processStatus: boolean;
    realizedPnl: number;
    unrealizedPnl: number;
    marketValue: number;
    margin: number;
    avail: number;
    addTime: string;
}
