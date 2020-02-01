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


InstrumentType = pylongfist.enum.InstrumentType
ExecType = pylongfist.enum.ExecType
Side = pylongfist.enum.Side
Offset = pylongfist.enum.Offset
BsFlag = pylongfist.enum.BsFlag
OrderStatus = pylongfist.enum.OrderStatus
Direction = pylongfist.enum.Direction
PriceType = pylongfist.enum.PriceType
VolumeCondition = pylongfist.enum.VolumeCondition
TimeCondition = pylongfist.enum.TimeCondition
OrderActionFlag = pylongfist.enum.OrderActionFlag
LedgerCategory = pylongfist.enum.LedgerCategory
HedgeFlag = pylongfist.enum.HedgeFlag
CommissionRateMode = pylongfist.enum.CommissionRateMode
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
