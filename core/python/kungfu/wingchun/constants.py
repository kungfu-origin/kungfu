from pykungfu import longfist as lf


class Source:
    SIM = "sim"
    CTP = "ctp"
    XTP = "xtp"
    OES = "oes"


class Exchange:
    SSE = "SSE"
    SZE = "SZE"
    SHFE = "SHFE"
    DCE = "DCE"
    CZCE = "CZCE"
    CFFEX = "CFFEX"
    INE = "INE"


class Region:
    CN = 'CN'
    HK = 'HK'


InstrumentType = lf.enums.InstrumentType
ExecType = lf.enums.ExecType
Side = lf.enums.Side
Offset = lf.enums.Offset
BsFlag = lf.enums.BsFlag
OrderStatus = lf.enums.OrderStatus
Direction = lf.enums.Direction
PriceType = lf.enums.PriceType
VolumeCondition = lf.enums.VolumeCondition
TimeCondition = lf.enums.TimeCondition
OrderActionFlag = lf.enums.OrderActionFlag
LedgerCategory = lf.enums.LedgerCategory
HedgeFlag = lf.enums.HedgeFlag
CommissionRateMode = lf.enums.CommissionRateMode
AllFinalOrderStatus = [OrderStatus.Filled, OrderStatus.Error, OrderStatus.PartialFilledNotActive, OrderStatus.Cancelled]

InstrumentTypeInStockAccount = [InstrumentType.Stock,
                                InstrumentType.Bond,
                                InstrumentType.Fund,
                                InstrumentType.StockOption,
                                InstrumentType.TechStock,
                                InstrumentType.Index,
                                InstrumentType.Repo]

ENUM_TYPES = [InstrumentType,
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
              HedgeFlag]
