
import pywingchun

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

LedgerCategory = pywingchun.constants.LedgerCategory
InstrumentType = pywingchun.constants.InstrumentType
ExecType = pywingchun.constants.ExecType
Side = pywingchun.constants.Side
Offset = pywingchun.constants.Offset
BsFlag = pywingchun.constants.BsFlag
OrderStatus = pywingchun.constants.OrderStatus
Direction = pywingchun.constants.Direction
PriceType = pywingchun.constants.PriceType
VolumeCondition = pywingchun.constants.VolumeCondition
TimeCondition = pywingchun.constants.TimeCondition
MsgType = pywingchun.constants.MsgType

WINGCHUN_ENUM_TYPES = [LedgerCategory, InstrumentType, ExecType, Side, Offset, BsFlag, OrderStatus,
                       Direction, PriceType, VolumeCondition, TimeCondition, MsgType]
InstrumentTypeInStock = [InstrumentType.Stock, InstrumentType.Fund, InstrumentType.TechStock]
AllFinalOrderStatus = [int(OrderStatus.Filled), int(OrderStatus.Error), int(OrderStatus.PartialFilledNotActive), int(OrderStatus.Cancelled)]




