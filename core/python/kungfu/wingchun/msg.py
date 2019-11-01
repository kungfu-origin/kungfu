
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

Registry.register(Quote, underscore(pywingchun.Quote.__name__), pywingchun.Quote, pywingchun.utils.write_quote, pywingchun.utils.get_quote)
Registry.register(Entrust, underscore(pywingchun.Entrust.__name__), pywingchun.Entrust, pywingchun.utils.write_entrust, pywingchun.utils.get_entrust)
Registry.register(Transaction, underscore(pywingchun.Transaction.__name__), pywingchun.Transaction, pywingchun.utils.write_transaction, pywingchun.utils.get_transaction)
Registry.register(Bar, underscore(pywingchun.Bar.__name__), pywingchun.Bar, pywingchun.utils.write_bar, pywingchun.utils.get_bar)
Registry.register(OrderInput, underscore(pywingchun.OrderInput.__name__), pywingchun.OrderInput, pywingchun.utils.write_order_input, pywingchun.utils.get_order_input)
Registry.register(OrderAction, underscore(pywingchun.OrderAction.__name__), pywingchun.OrderAction, pywingchun.utils.write_order_action, pywingchun.utils.get_order_action)
Registry.register(Order, underscore(pywingchun.Order.__name__), pywingchun.Order, pywingchun.utils.write_order, pywingchun.utils.get_order)
Registry.register(Trade, underscore(pywingchun.Trade.__name__), pywingchun.Trade, pywingchun.utils.write_trade, pywingchun.utils.get_trade)
Registry.register(Position, underscore(pywingchun.Position.__name__), pywingchun.Position, pywingchun.utils.write_position, pywingchun.utils.get_position)
Registry.register(Asset, underscore(pywingchun.Asset.__name__), pywingchun.Asset, pywingchun.utils.write_asset, pywingchun.utils.get_asset)
Registry.register(PositionDetail, underscore(pywingchun.PositionDetail.__name__), pywingchun.PositionDetail, pywingchun.utils.write_position_detail, pywingchun.utils.get_position_detail)
Registry.register(Instrument, underscore(pywingchun.Instrument.__name__), pywingchun.Instrument, pywingchun.utils.write_instrument, pywingchun.utils.get_instrument)







