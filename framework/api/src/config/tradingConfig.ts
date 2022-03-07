const isEnglish = process.env.LANG_ENV === 'en';

import {
  InstrumentTypeEnum,
  InstrumentTypes,
  ProcessStatusTypes,
  HedgeFlagEnum,
  PriceTypeEnum,
  VolumeConditionEnum,
  TimeConditionEnum,
  CommissionModeEnum,
  OffsetEnum,
  SideEnum,
  DirectionEnum,
  KfCategoryEnum,
  OrderStatusEnum,
  BrokerStateStatusTypes,
  FutureArbitrageCodeEnum,
} from '../typings/enums';

import { Pm2ProcessStatusTypes } from '../utils/processUtils';

export const Pm2ProcessStatus: Record<
  Pm2ProcessStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ['online']: {
    name: isEnglish ? 'Running' : '运行中',
    color: 'kf-color-running',
    level: 1,
  },
  ['stopping']: {
    name: isEnglish ? 'Stopping' : '停止中',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['stopped']: {
    name: isEnglish ? 'Stopped' : '已停止',
    color: 'kf-color-waiting',
    level: 0,
  },
  ['launching']: {
    name: isEnglish ? 'Launching' : '启动中',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['errored']: {
    name: isEnglish ? 'Errored' : '错误',
    color: 'kf-color-error',
    level: -1,
  },
  ['waiting restart']: {
    name: isEnglish ? 'Stopped' : '待重启',
    color: 'kf-color-waiting',
    level: 0,
  },
  ['one-launch-status']: {
    name: isEnglish ? 'Launching' : '启动中',
    color: 'kf-color-waiting',
    level: 1,
  },
};

export const BrokerStateStatus: Record<
  BrokerStateStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ['Pending']: {
    name: isEnglish ? 'Pending' : '等待中',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['Idle']: {
    name: isEnglish ? 'Idle' : '无数据',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['DisConnected']: {
    name: isEnglish ? 'Disconnected' : '已断开',
    color: 'kf-color-error',
    level: -1,
  },
  ['Connected']: {
    name: isEnglish ? 'Connected' : '已连接',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['LoggedIn']: {
    name: isEnglish ? 'LoggedIn' : '已登录',
    color: 'kf-color-waiting',
    level: 1,
  },
  ['LoginFailed']: {
    name: isEnglish ? 'LoginFailed' : '登录失败',
    color: 'kf-color-error',
    level: -1,
  },
  ['Ready']: {
    name: isEnglish ? 'Ready' : '就绪',
    color: 'kf-color-running',
    level: 2,
  },
};

export const AppStateStatus: Record<
  ProcessStatusTypes,
  KungfuApi.KfTradeValueCommonData
> = {
  ...Pm2ProcessStatus,
  ...BrokerStateStatus,
};

export const KfCategory: Record<
  KfCategoryEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [KfCategoryEnum.system]: {
    name: isEnglish ? 'System' : '系统服务',
    color: 'red',
    level: 100,
  },
  [KfCategoryEnum.md]: {
    name: isEnglish ? 'Md' : '行情源',
    color: 'orange',
    level: 90,
  },
  [KfCategoryEnum.td]: {
    name: isEnglish ? 'Td' : '交易账户',
    color: 'blue',
    level: 80,
  },
  [KfCategoryEnum.strategy]: {
    name: isEnglish ? 'Strategy' : '策略',
    color: 'cyan',
    level: 70,
  },
};

export const Offset: Record<OffsetEnum, KungfuApi.KfTradeValueCommonData> = {
  [OffsetEnum.Open]: { name: isEnglish ? 'Open' : '开', color: 'red' },
  [OffsetEnum.Close]: { name: isEnglish ? 'Close' : '平', color: 'green' },
  [OffsetEnum.CloseToday]: {
    name: isEnglish ? 'CloseToday' : '平今',
    color: 'green',
  },
  [OffsetEnum.CloseYest]: {
    name: isEnglish ? 'CloseYest' : '平昨',
    color: 'green',
  },
};

export const Side: Record<SideEnum, KungfuApi.KfTradeValueCommonData> = {
  [SideEnum.Buy]: { name: isEnglish ? 'Buy' : '买', color: 'red' },
  [SideEnum.Sell]: { name: isEnglish ? 'Sell' : '卖', color: 'green' },
  [SideEnum.Lock]: { name: isEnglish ? 'Lock' : '锁仓', color: 'orange' },
  [SideEnum.Unlock]: { name: isEnglish ? 'Unlock' : '解锁', color: 'blue' },
  [SideEnum.Exec]: { name: isEnglish ? 'Exec' : '行权', color: 'blue' },
  [SideEnum.Drop]: { name: isEnglish ? 'Drop' : '放弃行权', color: 'green' },
  [SideEnum.Purchase]: {
    name: isEnglish ? 'Purchase' : '申购',
    color: 'red',
  },
  [SideEnum.Redemption]: {
    name: isEnglish ? 'Redemption' : '赎回',
    color: 'green',
  },
  [SideEnum.Split]: { name: isEnglish ? 'Split' : '拆分', color: 'red' },
  [SideEnum.Merge]: { name: isEnglish ? 'Merge' : '合并', color: 'green' },
  [SideEnum.MarginTrade]: {
    name: isEnglish ? 'MarginTrade' : '融资买入',
    color: 'red',
  },
  [SideEnum.ShortSell]: {
    name: isEnglish ? 'ShortSell' : '融券卖出',
    color: 'green',
  },
  [SideEnum.RepayMargin]: {
    name: isEnglish ? 'RepayMargin' : '卖券还款',
    color: 'green',
  },
  [SideEnum.RepayStock]: {
    name: isEnglish ? 'RepayStock' : '买券还券',
    color: 'red',
  },
  [SideEnum.CashRepayMargin]: {
    name: isEnglish ? 'CashRepayMargin' : '现金还款',
    color: 'orange',
  },
  [SideEnum.StockRepayStock]: {
    name: isEnglish ? 'StockRepayStock' : '现券还券',
    color: 'blue',
  },
  [SideEnum.SurplusStockTransfer]: {
    name: isEnglish ? 'SurplusStockTransfer' : '余券划转',
    color: 'blue',
  },
  [SideEnum.GuaranteeStockTransferIn]: {
    name: isEnglish ? 'GuaranteeStockTransferIn' : '担保品转入',
    color: 'red',
  },
  [SideEnum.GuaranteeStockTransferOut]: {
    name: isEnglish ? 'GuaranteeStockTransferOut' : '担保品转出',
    color: 'green',
  },
};

export const OrderStatus: Record<
  OrderStatusEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [OrderStatusEnum.Unknown]: {
    name: isEnglish ? 'Unknown' : '未知',
    color: 'default',
  },
  [OrderStatusEnum.Submitted]: {
    name: isEnglish ? 'Submitted' : '已提交',
    color: 'default',
  },
  [OrderStatusEnum.Pending]: {
    name: isEnglish ? 'Pending' : '等待中',
    color: 'default',
  },
  [OrderStatusEnum.Cancelled]: {
    name: isEnglish ? 'Cancelled' : '已撤单',
    color: 'default',
  },
  [OrderStatusEnum.Error]: {
    name: isEnglish ? 'Error' : '错误',
    color: 'red',
  },
  [OrderStatusEnum.Filled]: {
    name: isEnglish ? 'Filled' : '已成交',
    color: 'green',
  },
  [OrderStatusEnum.PartialFilledNotActive]: {
    name: isEnglish ? 'PartialFilledNotActive' : '部分撤单',
    color: 'green',
  },
  [OrderStatusEnum.PartialFilledActive]: {
    name: isEnglish ? 'PartialFilledActive' : '正在交易',
    color: 'default',
  },
  [OrderStatusEnum.Lost]: {
    name: isEnglish ? 'Lost' : '丢失',
    color: 'default',
  },
};

export const UnfinishedOrderStatus = [
  OrderStatusEnum.Pending,
  OrderStatusEnum.Submitted,
  OrderStatusEnum.PartialFilledActive,
];

export const Direction: Record<
  DirectionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [DirectionEnum.Long]: { name: isEnglish ? 'Long' : '多', color: 'red' },
  [DirectionEnum.Short]: { name: isEnglish ? 'Short' : '空', color: 'green' },
};

export const PriceType: Record<
  PriceTypeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [PriceTypeEnum.Limit]: { name: isEnglish ? 'Limit' : '[Limit] 限价' },
  [PriceTypeEnum.Market]: { name: isEnglish ? 'Market' : '[Any] 市价' },
  [PriceTypeEnum.FakBest5]: {
    name: isEnglish
      ? 'FakBest5'
      : '[FakBest5] 上海深圳最优五档即时成交剩余撤销, 不需要报价',
  },
  [PriceTypeEnum.ForwardBest]: {
    name: isEnglish ? 'ForwardBest' : '[ForwardBest] 深圳本方方最优价格申报',
  },
  [PriceTypeEnum.ReverseBest]: {
    name: isEnglish
      ? 'ReverseBest'
      : '[ReverseBest] 上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报, 不需要报价',
  },
  [PriceTypeEnum.Fak]: {
    name: isEnglish ? 'Fak' : '[Fak] 深圳即时成交剩余撤销',
  },
  [PriceTypeEnum.Fok]: {
    name: isEnglish ? 'Fok' : '[Fok] 深圳市价全额成交或者撤销',
  },
  [PriceTypeEnum.Unknown]: { name: isEnglish ? 'UnKnown' : '未知' },
};

export const HedgeFlag: Record<
  HedgeFlagEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [HedgeFlagEnum.Speculation]: { name: isEnglish ? 'Speculation' : '投机' },
  [HedgeFlagEnum.Arbitrage]: { name: isEnglish ? 'Arbitrage' : '套利' },
  [HedgeFlagEnum.Hedge]: { name: isEnglish ? 'Hedge' : '套保' },
  [HedgeFlagEnum.Covered]: { name: isEnglish ? 'Covered' : '备兑' },
};

export const VolumeCondition: Record<
  VolumeConditionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [VolumeConditionEnum.Any]: { name: isEnglish ? 'Any' : '任意' },
  [VolumeConditionEnum.Min]: { name: isEnglish ? 'Min' : '最小' },
  [VolumeConditionEnum.All]: { name: isEnglish ? 'All' : '全部' },
};

export const TimeCondition: Record<
  TimeConditionEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [TimeConditionEnum.IOC]: { name: isEnglish ? 'IOC' : 'IOC' },
  [TimeConditionEnum.GFD]: { name: isEnglish ? 'GFD' : 'GFD' },
  [TimeConditionEnum.GTC]: { name: isEnglish ? 'GTC' : 'GTC' },
};

export const CommissionMode: Record<
  CommissionModeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [CommissionModeEnum.ByAmount]: { name: isEnglish ? 'ByAmount' : '按金额' },
  [CommissionModeEnum.ByVolume]: { name: isEnglish ? 'ByVolume' : '按手数' },
};

export const InstrumentType: Record<
  InstrumentTypeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [InstrumentTypeEnum.unknown]: {
    name: isEnglish ? 'Unknown' : '未知',
    color: 'default',
  },
  [InstrumentTypeEnum.stock]: {
    name: isEnglish ? 'Stock' : '普通股票',
    color: 'orange',
    level: 10,
  },
  [InstrumentTypeEnum.future]: {
    name: isEnglish ? 'Future' : '期货',
    color: 'red',
    level: 10,
  },
  [InstrumentTypeEnum.bond]: {
    name: isEnglish ? 'Bond' : '债券',
    color: 'pink',
  },
  [InstrumentTypeEnum.stockoption]: {
    name: isEnglish ? 'StockOption' : '股票期权',
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.fund]: {
    name: isEnglish ? 'Fund' : '基金',
    color: 'purple',
  },
  [InstrumentTypeEnum.techstock]: {
    name: isEnglish ? 'TechStock' : '科创板股票',
    color: 'blue',
  },
  [InstrumentTypeEnum.index]: {
    name: isEnglish ? 'Index' : '指数',
    color: 'purple',
  },
  [InstrumentTypeEnum.repo]: {
    name: isEnglish ? 'Repo' : '回购',
    color: 'red',
  },
  [InstrumentTypeEnum.crypto]: {
    name: isEnglish ? 'Crypto' : '数字货币',
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.cryptofuture]: {
    name: isEnglish ? 'CryptoFuture' : '数字货币合约',
    color: 'blue',
    level: 10,
  },
  [InstrumentTypeEnum.simu]: {
    name: isEnglish ? 'KungFuSimu' : '功夫模拟',
    color: 'green',
    level: 10,
  },
};

export const ShotableInstrumentTypes = [
  InstrumentTypeEnum.future,
  InstrumentTypeEnum.stockoption,
  InstrumentTypeEnum.cryptofuture,
  InstrumentTypeEnum.simu,
];

export const AbleSubscribeInstrumentTypesBySourceType: Record<
  InstrumentTypes,
  InstrumentTypeEnum[]
> = {
  unknown: [InstrumentTypeEnum.unknown],

  stock: [
    InstrumentTypeEnum.stock,
    InstrumentTypeEnum.bond,
    InstrumentTypeEnum.fund,
    InstrumentTypeEnum.techstock,
    InstrumentTypeEnum.index,
    InstrumentTypeEnum.repo,
  ],

  future: [InstrumentTypeEnum.future],

  bond: [InstrumentTypeEnum.bond],

  stockoption: [InstrumentTypeEnum.stockoption],

  fund: [InstrumentTypeEnum.fund],

  techstock: [InstrumentTypeEnum.techstock],

  index: [InstrumentTypeEnum.index],

  repo: [InstrumentTypeEnum.repo],

  crypto: [InstrumentTypeEnum.crypto],

  cryptofuture: [InstrumentTypeEnum.cryptofuture],

  simu: [
    InstrumentTypeEnum.stock,
    InstrumentTypeEnum.future,
    InstrumentTypeEnum.bond,
    InstrumentTypeEnum.stockoption,
    InstrumentTypeEnum.fund,
    InstrumentTypeEnum.techstock,
    InstrumentTypeEnum.index,
    InstrumentTypeEnum.repo,
    InstrumentTypeEnum.crypto,
    InstrumentTypeEnum.cryptofuture,
  ],
};

export const ExchangeIds: Record<string, KungfuApi.KfTradeValueCommonData> = {
  SSE: {
    name: isEnglish ? 'SSE' : '上交所',
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  SZE: {
    name: isEnglish ? 'SZE' : '深交所',
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  BSE: {
    name: isEnglish ? 'BSE' : '北交所',
    color: InstrumentType[InstrumentTypeEnum.stock].color,
  },
  SHFE: {
    name: isEnglish ? 'SHFE' : '上期所',
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  DCE: {
    name: isEnglish ? 'DCE' : '大商所',
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CZCE: {
    name: isEnglish ? 'CZCE' : '郑商所',
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  CFFEX: {
    name: isEnglish ? 'CFFEX' : '中金所',
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
  INE: {
    name: isEnglish ? 'INE' : '能源中心',
    color: InstrumentType[InstrumentTypeEnum.future].color,
  },
};

export const FutureArbitrageCodes: Record<
  FutureArbitrageCodeEnum,
  KungfuApi.KfTradeValueCommonData
> = {
  [FutureArbitrageCodeEnum.SP]: {
    name: isEnglish ? 'SP' : '郑商所 跨期 SP',
  },
  [FutureArbitrageCodeEnum.SPC]: {
    name: isEnglish ? 'SPC' : '郑商所 跨品种 SPC',
  },
  [FutureArbitrageCodeEnum.SPD]: {
    name: isEnglish ? 'SPD' : '大商所 跨期 SPD',
  },
  [FutureArbitrageCodeEnum.IPS]: {
    name: isEnglish ? 'IPS' : '大商所 跨品种 IPS',
  },
};

export const SystemProcessName: Record<
  string,
  KungfuApi.KfTradeValueCommonData
> = {
  master: { name: '主控进程' },
  ledger: { name: '计算服务' },
  archive: { name: '归档服务' },
};
