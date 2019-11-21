
import pywingchun
from kungfu.msg import Registry
from kungfu.msg.utils import underscore

Quote = 101
Entrust = 102
Transaction = 103
Bar = 110
OrderInput = 201
OrderAction = 202
Order = 203
Trade = 204
Position = 205
Asset = 206
AssetSnapshot = 207
PositionDetail = 208
Instrument = 209
AlgoOrderInput = 210
AlgoOrderReport = 211
AlgoOrderModify = 212
OrderActionError = 213
Subscribe = 302
NewOrderSingle = 353
CancelOrder = 354
CancelAllOrder = 355
BrokerStateRefresh = 400
BrokerState = 401
QryAsset = 402
PublishAllAssetInfo = 403
RemoveStrategy = 404
Calendar = 601
PositionEnd = 800
PositionDetailEnd = 801
InstrumentEnd = 802

Registry.register(Quote, underscore(pywingchun.Quote.__name__), pywingchun.Quote)
Registry.register(Entrust, underscore(pywingchun.Entrust.__name__), pywingchun.Entrust)
Registry.register(Transaction, underscore(pywingchun.Transaction.__name__), pywingchun.Transaction)
Registry.register(Bar, underscore(pywingchun.Bar.__name__), pywingchun.Bar)
Registry.register(OrderInput, underscore(pywingchun.OrderInput.__name__), pywingchun.OrderInput)
Registry.register(OrderAction, underscore(pywingchun.OrderAction.__name__), pywingchun.OrderAction)
Registry.register(OrderActionError, underscore(pywingchun.OrderActionError.__name__), pywingchun.OrderActionError)
Registry.register(Order, underscore(pywingchun.Order.__name__), pywingchun.Order)
Registry.register(Trade, underscore(pywingchun.Trade.__name__), pywingchun.Trade)
Registry.register(Position, underscore(pywingchun.Position.__name__), pywingchun.Position)
Registry.register(Asset, underscore(pywingchun.Asset.__name__), pywingchun.Asset)
Registry.register(PositionDetail, underscore(pywingchun.PositionDetail.__name__), pywingchun.PositionDetail)
Registry.register(Instrument, underscore(pywingchun.Instrument.__name__), pywingchun.Instrument)
Registry.register(PositionEnd,underscore(pywingchun.PositionEnd.__name__), pywingchun.PositionEnd)
Registry.register(AlgoOrderInput, "algo_order_input", str)






