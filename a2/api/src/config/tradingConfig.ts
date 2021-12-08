const isEnglish = false;

import {
    KfTradeValueCommonData,
    InstrumentTypeEnum,
    StateStatusEnum,
    HedgeFlagEnum,
    PriceTypeEnum,
    VolumeConditionEnum,
    TimeConditionEnum,
    CommissionModeEnum,
    OffsetEnum,
    SideEnum,
    DirectionEnum,
    KfCategoryEnum,
} from '@kungfu-trader/kungfu-js-api/typings';

export const StateStatusConfig: Record<
    StateStatusEnum,
    KfTradeValueCommonData
> = {
    [StateStatusEnum.online]: {
        name: isEnglish ? 'Running' : '运行中',
        color: 'kf-color-running',
        level: 1,
    },
    [StateStatusEnum.stopping]: {
        name: isEnglish ? 'Stopping' : '停止中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.stopped]: {
        name: isEnglish ? 'Stopped' : '已停止',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.launching]: {
        name: isEnglish ? 'Launching' : '启动中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.errored]: {
        name: isEnglish ? 'Errored' : '错误',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusEnum.waitingrestart]: {
        name: isEnglish ? 'Stopped' : '待重启',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.Pending]: {
        name: isEnglish ? 'Pending' : '等待中',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.Idle]: {
        name: isEnglish ? 'Idle' : '无数据',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.DisConnected]: {
        name: isEnglish ? 'Disconnected' : '已断开',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusEnum.Connected]: {
        name: isEnglish ? 'Connected' : '已连接',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.LoggedIn]: {
        name: isEnglish ? 'LoggedIn' : '已登录',
        color: 'kf-color-waiting',
        level: 0,
    },
    [StateStatusEnum.LoginFailed]: {
        name: isEnglish ? 'LoginFailed' : '登录失败',
        color: 'kf-color-error',
        level: -1,
    },
    [StateStatusEnum.Ready]: {
        name: isEnglish ? 'Ready' : '就绪',
        color: 'kf-color-running',
        level: 1,
    },
    [StateStatusEnum.Unknown]: {
        name: '--',
        color: 'kf-color-waiting',
        level: 0,
    },
};

export const Offset: Record<OffsetEnum, KfTradeValueCommonData> = {
    [OffsetEnum.Open]: { name: isEnglish ? 'Open' : '开' },
    [OffsetEnum.Close]: { name: isEnglish ? 'Close' : '平' },
    [OffsetEnum.CloseToday]: { name: isEnglish ? 'CloseToday' : '平今' },
    [OffsetEnum.CloseYest]: { name: isEnglish ? 'CloseYest' : '平昨' },
};

export const Side: Record<SideEnum, KfTradeValueCommonData> = {
    [SideEnum.Buy]: { name: isEnglish ? 'Buy' : '买' },
    [SideEnum.Sell]: { name: isEnglish ? 'Sell' : '卖' },
    [SideEnum.Lock]: { name: isEnglish ? 'Lock' : '锁仓' },
    [SideEnum.Unlock]: { name: isEnglish ? 'Unlock' : '解锁' },
    [SideEnum.Exec]: { name: isEnglish ? 'Exec' : '行权' },
    [SideEnum.Drop]: { name: isEnglish ? 'Drop' : '放弃行权' },
    [SideEnum.Purchase]: { name: isEnglish ? 'Purchase' : '申购' },
    [SideEnum.Redemption]: { name: isEnglish ? 'Redemption' : '赎回' },
    [SideEnum.Split]: { name: isEnglish ? 'Split' : '拆分' },
    [SideEnum.Merge]: { name: isEnglish ? 'Merge' : '合并' },
    [SideEnum.MarginTrade]: { name: isEnglish ? 'MarginTrade' : '融资买入' },
    [SideEnum.ShortSell]: { name: isEnglish ? 'ShortSell' : '融券卖出' },
    [SideEnum.RepayMargin]: { name: isEnglish ? 'RepayMargin' : '卖券还款' },
    [SideEnum.RepayStock]: { name: isEnglish ? 'RepayStock' : '买券还券' },
    [SideEnum.CashRepayMargin]: {
        name: isEnglish ? 'CashRepayMargin' : '现金还款',
    },
    [SideEnum.StockRepayStock]: {
        name: isEnglish ? 'StockRepayStock' : '现券还券',
    },
    [SideEnum.SurplusStockTransfer]: {
        name: isEnglish ? 'SurplusStockTransfer' : '余券划转',
    },
    [SideEnum.GuaranteeStockTransferIn]: {
        name: isEnglish ? 'GuaranteeStockTransferIn' : '担保品转入',
    },
    [SideEnum.GuaranteeStockTransferOut]: {
        name: isEnglish ? 'GuaranteeStockTransferOut' : '担保品转出',
    },
};

export const Direction: Record<DirectionEnum, KfTradeValueCommonData> = {
    [DirectionEnum.Long]: { name: isEnglish ? 'Long' : '多' },
    [DirectionEnum.Short]: { name: isEnglish ? 'Short' : '空' },
};

export const PriceType: Record<PriceTypeEnum, KfTradeValueCommonData> = {
    [PriceTypeEnum.Limit]: { name: isEnglish ? 'Limit' : '[Limit] 限价' },
    [PriceTypeEnum.Market]: { name: isEnglish ? 'Market' : '[Any] 市价' },
    [PriceTypeEnum.FakBest5]: {
        name: isEnglish
            ? 'FakBest5'
            : '[FakBest5] 上海深圳最优五档即时成交剩余撤销，不需要报价',
    },
    [PriceTypeEnum.ForwardBest]: {
        name: isEnglish
            ? 'ForwardBest'
            : '[ForwardBest] 深圳本方方最优价格申报',
    },
    [PriceTypeEnum.ReverseBest]: {
        name: isEnglish
            ? 'ReverseBest'
            : '[ReverseBest] 上海最优五档即时成交剩余转限价，深圳对手方最优价格申报，不需要报价',
    },
    [PriceTypeEnum.Fak]: {
        name: isEnglish ? 'Fak' : '[Fak] 深圳即时成交剩余撤销',
    },
    [PriceTypeEnum.Fok]: {
        name: isEnglish ? 'Fok' : '[Fok] 深圳市价全额成交或者撤销',
    },
    [PriceTypeEnum.Unknown]: { name: isEnglish ? 'UnKnown' : '未知' },
};

export const HedgeFlag: Record<HedgeFlagEnum, KfTradeValueCommonData> = {
    [HedgeFlagEnum.Speculation]: { name: isEnglish ? 'Speculation' : '投机' },
    [HedgeFlagEnum.Arbitrage]: { name: isEnglish ? 'Arbitrage' : '套利' },
    [HedgeFlagEnum.Hedge]: { name: isEnglish ? 'Hedge' : '套保' },
    [HedgeFlagEnum.Covered]: { name: isEnglish ? 'Covered' : '备兑' },
};

export const VolumeCondition: Record<
    VolumeConditionEnum,
    KfTradeValueCommonData
> = {
    [VolumeConditionEnum.Any]: { name: isEnglish ? 'Any' : '任意' },
    [VolumeConditionEnum.Min]: { name: isEnglish ? 'Min' : '最小' },
    [VolumeConditionEnum.All]: { name: isEnglish ? 'All' : '全部' },
};

export const TimeCondition: Record<TimeConditionEnum, KfTradeValueCommonData> =
    {
        [TimeConditionEnum.IOC]: { name: isEnglish ? 'IOC' : 'IOC' },
        [TimeConditionEnum.GFD]: { name: isEnglish ? 'GFD' : 'GFD' },
        [TimeConditionEnum.GTC]: { name: isEnglish ? 'GTC' : 'GTC' },
    };

export const CommissionMode: Record<
    CommissionModeEnum,
    KfTradeValueCommonData
> = {
    [CommissionModeEnum.ByAmount]: { name: isEnglish ? 'ByAmount' : '按金额' },
    [CommissionModeEnum.ByVolume]: { name: isEnglish ? 'ByVolume' : '按手数' },
};

export const InstrumentType: Record<
    InstrumentTypeEnum,
    KfTradeValueCommonData
> = {
    [InstrumentTypeEnum.Unknown]: {
        name: isEnglish ? 'Unknown' : '未知',
        color: 'default',
    },
    [InstrumentTypeEnum.Stock]: {
        name: isEnglish ? 'Stock' : '普通股票',
        color: 'orange',
        level: 10,
    },
    [InstrumentTypeEnum.Future]: {
        name: isEnglish ? 'Future' : '期货',
        color: 'pink',
        level: 10,
    },
    [InstrumentTypeEnum.Bond]: {
        name: isEnglish ? 'Bond' : '债券',
        color: 'red',
    },
    [InstrumentTypeEnum.StockOption]: {
        name: isEnglish ? 'StockOption' : '股票期权',
        color: 'blue',
        level: 10,
    },
    [InstrumentTypeEnum.Fund]: {
        name: isEnglish ? 'Fund' : '基金',
        color: 'purple',
    },
    [InstrumentTypeEnum.TechStock]: {
        name: isEnglish ? 'TechStock' : '科创板股票',
        color: 'orange',
    },
    [InstrumentTypeEnum.Index]: {
        name: isEnglish ? 'Index' : '指数',
        color: 'purple',
    },
    [InstrumentTypeEnum.Repo]: {
        name: isEnglish ? 'Repo' : '回购',
        color: 'red',
    },
    [InstrumentTypeEnum.Simu]: {
        name: isEnglish ? 'Simu' : '模拟',
        color: 'green',
        level: 10,
    },
};

export const ExchangeIds: Record<string, KfTradeValueCommonData> = {
    SSE: {
        name: isEnglish ? 'SSE' : '上交所',
        color: InstrumentType[InstrumentTypeEnum.Stock].color,
    },
    SZE: {
        name: isEnglish ? 'SZE' : '深交所',
        color: InstrumentType[InstrumentTypeEnum.Stock].color,
    },
    BSE: {
        name: isEnglish ? 'BSE' : '北交所',
        color: InstrumentType[InstrumentTypeEnum.Stock].color,
    },
    SHFE: {
        name: isEnglish ? 'SHFE' : '上期所',
        color: InstrumentType[InstrumentTypeEnum.Future].color,
    },
    DCE: {
        name: isEnglish ? 'DCE' : '大商所',
        color: InstrumentType[InstrumentTypeEnum.Future].color,
    },
    CZCE: {
        name: isEnglish ? 'CZCE' : '郑商所',
        color: InstrumentType[InstrumentTypeEnum.Future].color,
    },
    CFFEX: {
        name: isEnglish ? 'CFFEX' : '中金所',
        color: InstrumentType[InstrumentTypeEnum.Future].color,
    },
    INE: {
        name: isEnglish ? 'INE' : '能源中心',
        color: InstrumentType[InstrumentTypeEnum.Future].color,
    },
};

export const Category: Record<KfCategoryEnum, KfTradeValueCommonData> = {
    [KfCategoryEnum.md]: { name: 'md' },
    [KfCategoryEnum.td]: { name: 'td' },
    [KfCategoryEnum.strategy]: { name: 'strategy' },
    [KfCategoryEnum.system]: { name: 'system' },
};
