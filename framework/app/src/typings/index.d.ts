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
interface ExtraOrderInput {
  side: SideEnum;
  offset?: OffsetEnum;
  volume: number | bigint;
  price: number;
  accountId?: string;
}

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

type KfTradingDataTableSelection = Record<
  string,
  {
    disabled?: boolean;
  }
>;

type BuiltinComponents =
  | 'Pos'
  | 'PosGlobal'
  | 'Order'
  | 'Trade'
  | 'Td'
  | 'Md'
  | 'Strategy'
  | 'TradingTask'
  | 'MarketData'
  | 'OrderBook'
  | 'MakeOrder'
  | 'FutureArbitrage'
  | 'BlockTrade';

// 各个面板组件是由 component 动态组件加载，下面是传入的 props 类型
// 由于 vue3.2.x 的 defineProps 目前不支持外部引入类型和全局类型作为泛型参数，如果组件内是 setup 语法糖的话就得手动 copy 类型写在组件内部
interface BuiltinComponentProps {
  propsMapByComponent?: {
    TradingTask?: {
      taskFilter?: (task: Pm2ProcessStatusDetail) => boolean;
      strategyFilter?: (strategyExtConfig: KungfuApi.KfExtConfig) => boolean;
    };
  };
}

declare module 'worker-loader!*' {
  class WebpackWorker extends Worker {
    constructor();
  }

  export = WebpackWorker;
}
