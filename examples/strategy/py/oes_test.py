import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

source = "oes"
account = "customer167"
ticker = "600000"
price = 11.0
volume = 1000
exchange = Exchange.SSE

def pre_start(context):
    context.add_account(source, account, 100000000.0)
    context.subscribe(source, [ticker], exchange)
    context.ordered = False
    context.canceled = False
    pass

def pre_run(context):
    context.add_timer(1 * 1e9, test_time_interval)
    context.add_timer(1, test_time_interval)
    pass

def test_time_interval(context):
    context.log.info('test time interval without event')

def test_time_interval(context, event):
    context.log.info('test time interval')

def on_quote(context, quote):
    context.log.info("[on_quote]")
    #context.log.info("[on_quote] (ticker){} (last_price){} (trading_day){}".format(quote.instrument_id, quote.last_price, quote.trading_day))
    if not context.ordered:
        rid = insert_order(context, quote.data_time)
        if rid > 0:
            context.log.info("[order] (rid){}".format(rid))
            context.ordered = True

def insert_order(context, nano):
    context.log.info("before insert order")
    order_id = context.insert_limit_order(ticker, exchange, account, price, volume, Side.Buy, Offset.Open)
    if order_id > 0:
        context.log.info("insert order")
        return order_id
        #context.log.warning("insert market order: [order_id] {} [account] {}".format(order_id, "15040900"))

def on_transaction(context, transaction):
    context.log_info("[on_transaction] {} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log_info("[on_entrust] {} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.warning('[on_order] (ticker){} (order_id){}, (status){}'.format(order.instrument_id, order.order_id, order.status))
    if not context.canceled:# order.status == OrderStatus.Submitted:
        rid = context.cancel_order(order.order_id)
        if rid > 0:
            context.canceled = True
            context.log.info("cancel order (rid){}".format(rid))

def on_trade(context, trade):
    context.log.warning('[on_trade] (ticker){} (order_id){}, (volume){}'.format(trade.instrument_id, trade.order_id, trade.volume))

def pre_quit(context):
    context.log.warning("pre quit")
