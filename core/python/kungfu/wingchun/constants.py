from pykungfu import longfist as pylongfist


class Source:
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


InstrumentType = pylongfist.enums.InstrumentType
ExecType = pylongfist.enums.ExecType
Side = pylongfist.enums.Side
Offset = pylongfist.enums.Offset
BsFlag = pylongfist.enums.BsFlag
OrderStatus = pylongfist.enums.OrderStatus
Direction = pylongfist.enums.Direction
PriceType = pylongfist.enums.PriceType
VolumeCondition = pylongfist.enums.VolumeCondition
TimeCondition = pylongfist.enums.TimeCondition
OrderActionFlag = pylongfist.enums.OrderActionFlag
LedgerCategory = pylongfist.enums.LedgerCategory
HedgeFlag = pylongfist.enums.HedgeFlag
CommissionRateMode = pylongfist.enums.CommissionRateMode
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
