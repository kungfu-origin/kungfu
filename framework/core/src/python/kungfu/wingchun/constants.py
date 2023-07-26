#  SPDX-License-Identifier: Apache-2.0

from pykungfu import longfist as lf


class Exchange:
    EXCHANGE_SSE = "SSE"  # 上交所
    EXCHANGE_SZE = "SZE"  # 深交所
    EXCHANGE_BSE = "BSE"  # 北交所
    EXCHANGE_GFEX = "GFEX"  # 广交所
    EXCHANGE_SHFE = "SHFE"  # 上期所
    EXCHANGE_DCE = "DCE"  # 大商所
    EXCHANGE_CZCE = "CZCE"  # 郑商所
    EXCHANGE_CFFEX = "CFFEX"  # 中金所
    EXCHANGE_INE = "INE"  # 上海能源中心
    EXCHANGE_BINANCE = "BINANCE"
    EXCHANGE_HB = "HB"

    # 全市场exchange id定义
    EXCHANGE_HK = "HK"  # 港股: 4（香港交易所）
    EXCHANGE_HK_FUTURE = "HKFUT"  # 港股期货: 5（香港交易所）
    EXCHANGE_US = "US"  # 美股: 29（纳斯达克交易所）
    EXCHANGE_US_FUTURE = "USFUT"  # 美期: 29（纳斯达克交易所）
    EXCHANGE_SGX = "SGX"  # 新加坡交易所股票
    EXCHANGE_SGX_FUTURE = "SGXFUT"  # 新加坡交易所期货
    EXCHANGE_EUR = "EUR"  # 欧洲交易所股票
    EXCHANGE_EUR_FUTURE = "EURFUT"  # 欧洲坡交易所期货
    EXCHANGE_LON = "LON"  # 英股: 伦敦证券交易所
    EXCHANGE_LON_FUTURE = "LONFUT"  # 英期
    EXCHANGE_AEX = "AEX"  # 荷股
    EXCHANGE_AEX_FUTURE = "AEXFUT"  # 荷期
    EXCHANGE_AUX = "AUX"  # 澳股
    EXCHANGE_AUX_FUTURE = "AUXFUT"  # 澳期
    EXCHANGE_HEXS = "HEXS"  # 芬股
    EXCHANGE_HEXS_FUTURE = "HEXSFUT"  # 芬期
    EXCHANGE_IDX = "IDX"  # 印尼股
    EXCHANGE_IDX_FUTURE = "IDXFUT"  # 印尼期
    EXCHANGE_KORC = "KORC"  # 韩碳所
    EXCHANGE_LME = "LME"  # 伦金所
    EXCHANGE_MYS = "MYS"  # 马来股
    EXCHANGE_MYS_FUTURE = "MYSFUT"  # 马来期
    EXCHANGE_ABB = "ABB"  # 美布告
    EXCHANGE_PRX = "PRX"  # 法股
    EXCHANGE_PRX_FUTURE = "PRXFUT"  # 法期
    EXCHANGE_SIX = "SIX"  # 瑞股
    EXCHANGE_SIX_FUTURE = "SIXFUT"  # 瑞期
    EXCHANGE_TAX = "TAX"  # 泰股
    EXCHANGE_TAX_FUTURE = "TAXFUT"  # 泰期
    EXCHANGE_JP = "JP"  # 日股
    EXCHANGE_JP_FUTURE = "JPFUT"  # 日期
    EXCHANGE_TSE = "TSE"  # 多股
    EXCHANGE_TSE_FUTURE = "TSEFUT"  # 多期
    EXCHANGE_XETRA = "XETRA"  # XETRA

    EXCHANGE_GLFX = "GLFX"  # 全球外汇: 41
    EXCHANGE_IPE = "IPE"  # LME\IPE: 45(LME（伦敦金属交易所）、ICE)
    EXCHANGE_CBOT = "CBOT"  # ES-CBOT: 62
    EXCHANGE_CEC = "CEC"  # ES-CEC: 63
    EXCHANGE_LIFE = "LIFE"  # ES-LIFE: 64
    EXCHANGE_MTIF = "MTIF"  # ES-MTIF: 65
    EXCHANGE_NYCE = "NYCE"  # ES-NYCE: 66、49
    EXCHANGE_CMX = "CMX"  # ES-CMX: 67
    EXCHANGE_NYME = "NYME"  # ES-NYME: 68
    EXCHANGE_SIME = "SIME"  # ES-SIME: 69
    EXCHANGE_CME = "CME"  # ES-CME: 70
    EXCHANGE_IMM = "IMM"  # ES-IMM: 71
    EXCHANGE_WIDX = "WIDX"  # ES-WIDX: 72
    EXCHANGE_FREX = "FREX"  # ES-FREX: 73
    EXCHANGE_METL = "METL"  # ES-METL: 74
    EXCHANGE_IPM = "IPM"  # 国际贵金属: 5000


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
