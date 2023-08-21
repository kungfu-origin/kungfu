#  SPDX-License-Identifier: Apache-2.0

from pykungfu import longfist as lf


class Exchange:
    SSE = "SSE"  # 上交所
    SZE = "SZE"  # 深交所
    BSE = "BSE"  # 北交所
    GFEX = "GFEX"  # 广交所
    SHFE = "SHFE"  # 上期所
    DCE = "DCE"  # 大商所
    CZCE = "CZCE"  # 郑商所
    CFFEX = "CFFEX"  # 中金所
    INE = "INE"  # 上海能源中心
    BINANCE = "BINANCE"
    HB = "HB"

    # 全市场exchange id定义
    HK = "HK"  # 港股: 4（香港交易所）
    HK_FUTURE = "HKFUT"  # 港股期货: 5（香港交易所）
    US = "US"  # 美股: 29（纳斯达克交易所）
    US_FUTURE = "USFUT"  # 美期: 29（纳斯达克交易所）
    SGX = "SGX"  # 新加坡交易所股票
    SGX_FUTURE = "SGXFUT"  # 新加坡交易所期货
    EUR = "EUR"  # 欧洲交易所股票
    EUR_FUTURE = "EURFUT"  # 欧洲坡交易所期货
    LON = "LON"  # 英股: 伦敦证券交易所
    LON_FUTURE = "LONFUT"  # 英期
    AEX = "AEX"  # 荷股
    AEX_FUTURE = "AEXFUT"  # 荷期
    AUX = "AUX"  # 澳股
    AUX_FUTURE = "AUXFUT"  # 澳期
    HEXS = "HEXS"  # 芬股
    HEXS_FUTURE = "HEXSFUT"  # 芬期
    IDX = "IDX"  # 印尼股
    IDX_FUTURE = "IDXFUT"  # 印尼期
    KORC = "KORC"  # 韩碳所
    LME = "LME"  # 伦金所
    MYS = "MYS"  # 马来股
    MYS_FUTURE = "MYSFUT"  # 马来期
    ABB = "ABB"  # 美布告
    PRX = "PRX"  # 法股
    PRX_FUTURE = "PRXFUT"  # 法期
    SIX = "SIX"  # 瑞股
    SIX_FUTURE = "SIXFUT"  # 瑞期
    TAX = "TAX"  # 泰股
    TAX_FUTURE = "TAXFUT"  # 泰期
    JP = "JP"  # 日股
    JP_FUTURE = "JPFUT"  # 日期
    TSE = "TSE"  # 多股
    TSE_FUTURE = "TSEFUT"  # 多期
    XETRA = "XETRA"  # XETRA

    GLFX = "GLFX"  # 全球外汇: 41
    IPE = "IPE"  # LME\IPE: 45(LME（伦敦金属交易所）、ICE)
    CBOT = "CBOT"  # ES-CBOT: 62
    CEC = "CEC"  # ES-CEC: 63
    LIFE = "LIFE"  # ES-LIFE: 64
    MTIF = "MTIF"  # ES-MTIF: 65
    NYCE = "NYCE"  # ES-NYCE: 66、49
    CMX = "CMX"  # ES-CMX: 67
    NYME = "NYME"  # ES-NYME: 68
    SIME = "SIME"  # ES-SIME: 69
    CME = "CME"  # ES-CME: 70
    IMM = "IMM"  # ES-IMM: 71
    WIDX = "WIDX"  # ES-WIDX: 72
    FREX = "FREX"  # ES-FREX: 73
    METL = "METL"  # ES-METL: 74
    IPM = "IPM"  # 国际贵金属: 5000


class Region:
    CN = "CN"
    HK = "HK"


InstrumentType = lf.enums.InstrumentType
ExecType = lf.enums.ExecType
Side = lf.enums.Side
Offset = lf.enums.Offset
BsFlag = lf.enums.BsFlag
OrderStatus = lf.enums.OrderStatus
Direction = lf.enums.Direction
PriceType = lf.enums.PriceType
PriceLevel = lf.enums.PriceLevel
VolumeCondition = lf.enums.VolumeCondition
TimeCondition = lf.enums.TimeCondition
OrderActionFlag = lf.enums.OrderActionFlag
LedgerCategory = lf.enums.LedgerCategory
HedgeFlag = lf.enums.HedgeFlag
CommissionRateMode = lf.enums.CommissionRateMode
MarketType = lf.enums.MarketType
SubscribeDataType = lf.enums.SubscribeDataType
SubscribeInstrumentType = lf.enums.SubscribeInstrumentType
StrategyState = lf.enums.StrategyState
HistoryDataType = lf.enums.HistoryDataType

BasketVolumeType = lf.enums.BasketVolumeType
BasketType = lf.enums.BasketType

AllFinalOrderStatus = [
    int(OrderStatus.Filled),
    int(OrderStatus.Error),
    int(OrderStatus.PartialFilledNotActive),
    int(OrderStatus.Cancelled),
]

InstrumentTypeInStockAccount = [
    InstrumentType.Stock,
    InstrumentType.Bond,
    InstrumentType.Fund,
    InstrumentType.StockOption,
    InstrumentType.TechStock,
    InstrumentType.Index,
    InstrumentType.Repo,
]

ENUM_TYPES = [
    InstrumentType,
    ExecType,
    Side,
    Offset,
    BsFlag,
    OrderStatus,
    Direction,
    PriceType,
    VolumeCondition,
    TimeCondition,
    OrderActionFlag,
    LedgerCategory,
    HedgeFlag,
    MarketType,
    SubscribeDataType,
    SubscribeInstrumentType,
]
