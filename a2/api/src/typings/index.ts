import { Pm2ProcessStatusTypes } from '../utils/processUtils';

export interface KfTradeValueCommonData {
    name: string;
    color?: AntInKungfuColorTypes;
    level?: number;
}

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

export type ProcessStatusTypes = Pm2ProcessStatusTypes | BrokerStateStatusTypes;

export type AntInKungfuColorTypes =
    | 'default'
    | 'orange'
    | 'yellow'
    | 'pink'
    | 'red'
    | 'blue'
    | 'purple'
    | 'cyan'
    | 'green'
    | 'red'
    | 'kf-color-running'
    | 'kf-color-waiting'
    | 'kf-color-error';

export enum LedgerCategoryEnum {
    Account,
    Strategy,
}

export type LedgerCategoryTypes = keyof typeof LedgerCategoryEnum;

export enum InstrumentTypeEnum {
    unknown,
    stock,
    future,
    bond,
    stockoption,
    fund,
    techstock,
    index,
    repo,
    crypto,
    cryptofuture,
    simu,
}

export type InstrumentTypes = keyof typeof InstrumentTypeEnum;

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
}

export type SideTypes = keyof typeof SideEnum;

export enum DirectionEnum {
    Long,
    Short,
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

export interface SourceData {
    name: string;
    type: InstrumentTypes[];
}

export type KfConfigItemSupportedTypes =
    | 'str'
    | 'password'
    | 'file' // string
    | 'process'
    | 'instrumentId' // search input
    | 'account' // select - string
    | 'source' // select - string
    | 'exchange' // select - string
    | 'select'
    | 'bool'
    | 'int'
    | 'float'
    | 'percent'
    | 'side' // select - number
    | 'offset' // select - number
    | 'direction' // select - number
    | 'priceType' // select - number
    | 'hedgeFlag' // select - number
    | 'volumeCondition' // select - number
    | 'timeCondition' // select - number
    | 'commissionMode' // select - number
    | 'instrumentType'; // select - number

export type KfConfigValue = string | number | boolean;

export interface KfSelectOption {
    value: string | number;
    label: string | number;
}

export interface KfConfigItem {
    key: string;
    name: string;
    type: KfConfigItemSupportedTypes;
    errMsg?: string;
    default?: KfConfigValue;
    required?: boolean;
    primary?: boolean;
    validator?: string[];
    options?: KfSelectOption[];
    data?: KfSelectOption[];
    args?: Array<{ key: string | number; value: string | number }>; // process
    target?: string; // process
    tip?: string;
}

export interface KfExtOriginConfig {
    key: string;
    name: string;
    config: {
        [key in KfCategoryTypes]?: {
            type?: Array<InstrumentTypes> | InstrumentTypes;
            settings: KfConfigItem[];
        };
    };
}

export type KfExtConfigs = {
    [key in KfCategoryTypes]?: {
        [extKey: string]: KfExtOriginConfig['config'][KfCategoryTypes];
    };
};

export interface SetKfConfigPayload {
    type: ModalChangeType;
    title: string;
    config: KfExtOriginConfig['config'][KfCategoryTypes];
    initValue?: Record<string, KfConfigValue>;
}

export enum KfCategoryEnum {
    md,
    td,
    strategy,
    system,
}
export type KfCategoryTypes = keyof typeof KfCategoryEnum;

export enum KfModeEnum {
    LIVE,
    DATA,
    REPLAY,
    BACKTEST,
}

export type KfModeTypes = keyof typeof KfModeEnum;

export interface KfLocationBase {
    group: string;
    name: string;
}

export type KfLocation = {
    category: KfCategoryTypes;
    mode: KfModeTypes;
} & KfLocationBase;

export type KfLocationOrigin = {
    category: KfCategoryEnum;
    mode: KfModeEnum;
} & KfLocationBase;

export type KfConfigOrigin = KfLocationOrigin & {
    location_uid: number;
    value: string;
};

export type KfConfig = KfLocation & {
    location_uid: number;
    value: string;
};
