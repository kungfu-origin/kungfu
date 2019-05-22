
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

class Side:
    Buy = "0"
    Sell = "1"


class Offset:
    Open = "0"
    Close = "1"
    CloseToday = "2"
    CloseYesterday = "3"


class OrderStatus:
    Unknown = "0"
    Submitted = "1"
    Pending = "2"
    Canceled = "3"
    Error = "4"
    Filled = "5"
    PartialFilledNotActive = "6"
    PartialFilledActive = "7"

    @classmethod
    def is_final_status(cls, status):
        if status == OrderStatus.Error or status == OrderStatus.Canceled or status == OrderStatus.Filled or status == OrderStatus.PartialFilledNotActive:
            return True
        else:
            return False

    @classmethod
    def to_str(cls, status):
        if status == OrderStatus.Submitted:
            return "Submitted"
        elif status == OrderStatus.Pending:
            return "Pending"
        elif status == OrderStatus.Canceled:
            return "Canceled"
        elif status == OrderStatus.Error:
            return "Error"
        elif status == OrderStatus.Filled:
            return "Filled"
        elif status == OrderStatus.PartialFilledNotActive:
            return "PartialFilledNotActive"
        elif status == OrderStatus.PartialFilledActive:
            return "PartialFilledActive"
        else:
            return "Unknown"

class Direction:
    Long = "0"
    Short = "1"


class MsgType:
    Quote = 101
    Entrust = 102
    Transaction = 103

    OrderInput = 201
    OrderAction = 202
    Order = 203
    Trade = 204
    Position = 205
    AccountInfo = 206
    Portfolio = 207
    AccountInfoByMin = 208
    PortfolioByMin = 209
    PositionDetail = 210
    SubPortfolioInfo = 211

    ReqLogin = 301
    Subscribe = 302
    ReqOrderInput = 303
    ReqOrderAction = 304

    RspLogin = 351
    RspSubscribe = 352
    RspOrderInput = 353
    RspOrderAction = 354

    GatewayState = 401

    AlgoOrderInput = 501
    AlgoOrderUpdate = 502

    SwitchDay = 601
    RspTradingDay = 602

    ReloadFutureInstrument = 701


class InstrumentType:
    Unknown = '0'
    Stock = '1'
    Future = '2'
    Bond = '3'
    StockOption = '4'


class Region:
    CN = 'CN'
    HK = 'HK'
