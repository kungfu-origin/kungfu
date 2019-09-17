
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

InstrumentType = pywingchun.constants.InstrumentType
InstrumentTypeInStock = [InstrumentType.Stock, InstrumentType.Fund, InstrumentType.TechStock]
Side = pywingchun.constants.Side
Offset = pywingchun.constants.Offset
OrderStatus = pywingchun.constants.OrderStatus
AllFinalOrderStatus = [int(OrderStatus.Filled), int(OrderStatus.Error), int(OrderStatus.PartialFilledNotActive), int(OrderStatus.Cancelled)]
Direction = pywingchun.constants.Direction
PriceType = pywingchun.constants.PriceType
VolumeCondition = pywingchun.constants.VolumeCondition
TimeCondition = pywingchun.constants.TimeCondition
MsgType = pywingchun.constants.MsgType
LedgerCategory = pywingchun.constants.LedgerCategory



