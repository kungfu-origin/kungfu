import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

source = "sim"
account = "test"
ticker = "600000"
price = 13.2
volume = 10000
exchange = Exchange.SSE

def pre_start(context):
    context.add_account(source, account, 100000000.0)
    context.subscribe(source, [ticker], exchange)

def pre_run(context):
    pass

def on_quote(context, quote):
    context.log.info("[on_quote] (ticker){} (last_price){} (trading_day){}".format(quote.instrument_id, quote.last_price, quote.trading_day))
    insert_order(context, quote.data_time)

def insert_order(context, nano):
    context.log.info("before insert order")
    #order_id = context.insert_limit_order(ticker, exchange, account, price, volume, Side.Buy, Offset.Open)
    order_id = context.insert_order(ticker, exchange, account, price, volume, PriceType.Limit, Side.Buy, Offset.Open, HedgeFlag.Speculation)
    if order_id > 0:
        context.log.info("insert order")
        #context.log.warning("insert market order: [order_id] {} [account] {}".format(order_id, "15040900"))

def on_transaction(context, transaction):
    #context.log_info("[on_transaction] {} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    #context.log_info("[on_entrust] {} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.warning('[on_order] (ticker){} (order_id){}, (status){}'.format(order.instrument_id, order.order_id, order.status))

def on_trade(context, trade):
    context.log.warning('[on_trade] (ticker){} (order_id){}, (volume){}'.format(trade.instrument_id, trade.order_id, trade.volume))

def pre_quit(context):
    context.log.warning("pre quit")
