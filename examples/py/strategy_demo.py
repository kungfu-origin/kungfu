
from kungfu.wingchun.constants import *
source = Source.XTP
exchange = Exchange.SSE

def init(context):
    context.add_md(source)
    context.add_account(source, "15011218", 100000000.0)
    context.add_account(source, "15040900", 100000000.0)
    context.subscribe(source, ["601988", "600000"], exchange, True)
    context.register_nanotime_callback(context.get_nano() + int(5* 1e9), insert_order)
    sub_portfolio = context.get_sub_portfolio_info("15040900")
    print(sub_portfolio)
    portfolio = context.get_portfolio_info()
    print(portfolio)

def pre_run(context):
    context.log.info("pre run strategy")
    pass


def insert_order(context, nano):

    order_id = context.insert_market_order("601988", exchange, "15040900", 200, Side.Buy, Offset.Open)
    context.log.info("insert market order: [order_id] %d [account] %s", order_id, "15040900")
    order_id = context.insert_limit_order("600000", exchange, "15011218", 12.8, 300, Side.Buy, Offset.Open)
    context.log.info("insert limit order: [order_id] %d [account] %s", order_id, "15011218")

def on_quote(context, quote):
    context.log.info("{} {}".format(quote.instrument_id, quote.trading_day))
    pass

def on_transaction(context, transaction):
    context.log.info("{} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log.info("{} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.info('rcv_time {} ticker {} order_id {}, status {}|{}'.format(order.rcv_time, order.instrument_id, order.order_id, order.status, OrderStatus.to_str(order.status)))

def on_trade(context, trade):
    context.log.info('rcv_time {} order_id {}'.format(trade.rcv_time, trade.order_id))

def pre_quit(context):
    context.log.info("pre quit")
