import { Pm2ProcessStatusTypes } from '../utils/processUtils';

export enum BrokerStateStatusEnum {
  Pending = 0,
  Idle = 1,
  DisConnected = 2,
  Connected = 3,
  LoggedIn = 4,
  LoginFailed = 5,
  Ready = 100,
}

export type BrokerStateStatusTypes = keyof typeof BrokerStateStatusEnum;

export type ProcessStatusTypes =
  | Pm2ProcessStatusTypes
  | BrokerStateStatusTypes
  | StrategyStateStatusTypes;

export enum StrategyStateStatusEnum {
  Normal,
  Warn,
  Error,
}

export type StrategyStateStatusTypes = keyof typeof StrategyStateStatusEnum;

export enum LedgerCategoryEnum {
  td = 0,
  strategy = 1,
}

export type LedgerCategoryTypes = keyof typeof LedgerCategoryEnum;

export enum InstrumentTypeEnum {
  unknown,
  stock,
  future,
  bond,
  stockoption,
  techstock,
  fund,
  index,
  repo,
  warrant,
  iopt,
  crypto,
  cryptofuture,
  multi,
}
export type InstrumentTypes = keyof typeof InstrumentTypeEnum;

export enum BasketVolumeTypeEnum {
  Unknown,
  Quantity,
  Proportion,
}

export type BasketVolumeTypes = keyof typeof BasketVolumeTypeEnum;

export enum PriceLevelEnum {
  Latest, // 最新价
  Sell5,
  Sell4,
  Sell3,
  Sell2,
  Sell1,
  Buy1,
  Buy2,
  Buy3,
  Buy4,
  Buy5,
  UpperLimitPrice, // 涨停价
  LowerLimitPrice, // 跌停价
  Unknown,
}

export type PriceLevelTypes = keyof typeof PriceLevelEnum;

export type StrategyExtTypes = 'trade' | 'default' | 'unknown';

export enum HedgeFlagEnum {
  Speculation,
  Arbitrage,
  Hedge,
  Covered,
}

export type HedgeFlagTypes = keyof typeof HedgeFlagEnum;

export enum PriceTypeEnum {
  Limit,
  Market,
  FakBest5,
  ForwardBest,
  ReverseBest,
  Fak,
  Fok,
  Unknown,
}

export type PriceTypes = keyof typeof PriceTypeEnum;

export enum VolumeConditionEnum {
  Any,
  Min,
  All,
}

export type VolumeConditionTypes = keyof typeof VolumeConditionEnum;

export enum TimeConditionEnum {
  IOC,
  GFD,
  GTC,
}

export type TimeConditionTypes = keyof typeof TimeConditionEnum;

export enum CommissionModeEnum {
  ByAmount,
  ByVolume,
}

export type CommissionModeTypes = keyof typeof CommissionModeEnum;

export enum OffsetEnum {
  Open,
  Close,
  CloseToday,
  CloseYest,
}

export type OffsetTypes = keyof typeof OffsetEnum;

export enum SideEnum {
  Buy,
  Sell,
  Lock,
  Unlock,
  Exec,
  Drop,
  Purchase,
  Redemption,
  Split,
  Merge,
  MarginTrade,
  ShortSell,
  RepayMargin,
  RepayStock,
  CashRepayMargin,
  StockRepayStock,
  SurplusStockTransfer,
  GuaranteeStockTransferIn,
  GuaranteeStockTransferOut,
  Unknown = 99,
}

export type SideTypes = keyof typeof SideEnum;

export enum DirectionEnum {
  Long,
  Short,
}

export enum UnderweightEnum {
  UnrestrictedShares,
  RestrictedShares,
}

export type DirectionTypes = keyof typeof DirectionEnum;

export enum OrderStatusEnum {
  Unknown,
  Submitted,
  Pending,
  Cancelled,
  Error,
  Filled,
  PartialFilledNotActive,
  PartialFilledActive,
  Lost,
}

export type OrderStatusTypes = keyof typeof OrderStatusEnum;

export enum BasketOrderStatusEnum {
  Unknown,
  Pending,
  PartialFilledNotActive, // 部分成交已结束
  PartialFilledActive, // 部分成交未结束
  Filled,
}

export enum KfCategoryEnum {
  md,
  td,
  strategy,
  system,
  daemon,
}

export type KfCategoryTypes = keyof typeof KfCategoryEnum;

export type KfUIExtLocatorTypes =
  | 'sidebar'
  | 'sidebar_footer'
  | 'board'
  | 'global_setting'
  | 'make_order'
  | 'trading_task_view';

export type KfExtConfigTypes = 'form' | '';

export enum KfModeEnum {
  live,
  data,
  replay,
  backtest,
}

export type KfModeTypes = keyof typeof KfModeEnum;

export enum HistoryDateEnum {
  naturalDate,
  tradingDate,
}

export enum OrderActionFlagEnum {
  Cancel,
}

export enum FutureArbitrageCodeEnum {
  SP = 'SP',
  SPC = 'SPC',
  SPD = 'SPD',
  IPS = 'IPS',
}

export enum SpaceTabSettingEnum {
  SPACES = 'SPACES',
  TABS = 'TABS',
}

export enum SpaceSizeSettingEnum {
  FOURINDENT = 'FOURINDENT',
  TWOINDENT = 'TWOINDENT',
}

export enum OrderInputKeyEnum {
  VOLUME = 'VOLUME',
  PRICE = 'PRICE',
}

export enum CurrencyEnum {
  Unknown,
  CNY, // 人民币
  HKD, // 港币
  USD, // 美元
  JPY, // 日元
  GBP, // 英镑
  EUR, // 欧元
  CNH, // 离岸人民币
  SGD, // 新加坡元
  MYR, // 马来西亚吉特
}

export enum FundTransEnum {
  Pending,
  Success,
  Error,
}

export enum FundTransTypeEnum {
  BetweenNodes = 'between_nodes',
  TrancIn = 'tranc_in',
  TrancOut = 'tranc_out',
}
