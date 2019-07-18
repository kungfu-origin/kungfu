from kungfu.wingchun.constants import *
source = "passive"
account = "test"
ticker = "600000"
price = 13.1
volume = 10000
exchange = Exchange.SSE

def init(context):
    context.add_md(source)
    context.add_account(source, account, 100000000.0)
    context.subscribe(source, ticker, exchange, True)
    context.register_nanotime_callback(context.get_nano() + int(5* 1e9), insert_order)

def pre_run(context):
    pass

def insert_order(context, nano):
    #order_id = context.insert_market_order(ticker, exchange, "15040900", 100, Side.Buy, Offset.Open)
    context.log.info("before insert order")
    order_id = context.insert_limit_order(ticker, exchange, account, price, volume, Side.Buy, Offset.Open)
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
    context.log.warning('[on_order] rcv_time {} ticker {} order_id {}, status {}|{}'.format(order.rcv_time, order.instrument_id, order.order_id, order.status, OrderStatus.to_str(order.status)))

def on_trade(context, trade):
    context.log.warning('[on_trade] rcv_time {} order_id {}'.format(trade.rcv_time, trade.order_id))

def pre_quit(context):
    context.log.warning("pre quit")
