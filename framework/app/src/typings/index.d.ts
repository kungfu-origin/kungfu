/* eslint-disable  @typescript-eslint/no-unused-vars */
/* eslint-disable  @typescript-eslint/no-explicit-any */
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
  minWidth?: number | string;
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

interface TriggerOrderBookUpdate {
  tag: 'orderBookUpdate';
  orderInput: InstrumentResolved | ExtraOrderInput;
}

interface TriggerMakeOrder {
  tag: 'makeOrder';
  orderInput: InstrumentResolved | ExtraOrderInput;
}

interface TriggerUpdateTdGroup {
  tag: 'update:tdGroup';
  tdGroups: KungfuApi.KfExtraLocation[];
}

interface TriggerUpdateTd {
  tag: 'update:td';
  tds: KungfuApi.KfConfig[];
}

interface TriggerUpdateMd {
  tag: 'update:md';
  mds: KungfuApi.KfConfig[];
}

interface TriggerUpdateStrategy {
  tag: 'update:strategy';
  strategys: KungfuApi.KfConfig[];
}

interface TriggerAddBoard {
  tag: 'addBoard';
  boardId: number;
}

type KfBusEvent =
  | ResizeEvent
  | ProcessStatusChangeEvent
  | MainProcessEvent
  | TradingDataUpdateEvent
  | TriggeOrderBook
  | TriggerOrderBookUpdate
  | TriggerMakeOrder
  | TriggerUpdateTdGroup
  | TriggerUpdateTd
  | TriggerUpdateMd
  | TriggerUpdateStrategy
  | TriggerAddBoard
  | ExportTradingDataEvent;

interface ExtraOrderInput {
  side: SideEnum;
  offset?: OffsetEnum;
  volume: number | bigint;
  price: number;
  accountId?: string;
}


interface FileProps {
    root?: Array;
    filePath?: string;
    ext?: string;
    isDir?: boolean
}
interface IValue {
    isDir: boolean
}
interface ICurrentFile {
    filePath?: string;
    value: IValue;
}
interface IFileNode {
    filePath: string;
    root?: string;
    children?: IFileNode;
    name: string;
}


interface ICodeSetting {
    tabSpaceType: 'Spaces' | 'Tabs';
    tabSpaceSize: '2' | '4'

}

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
  sorter?: (a: any, b: any) => number;
}


declare module 'worker-loader!*' {
  class WebpackWorker extends Worker {
    constructor();
  }

  export = WebpackWorker;
}
