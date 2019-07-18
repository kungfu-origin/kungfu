import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

source = Source.XTP
exchange = Exchange.SSE


def test_timer(context, event):
    context.log.info('test timer')

def test_time_interval(context, event):
    context.log.info('test time interval')


def pre_start(context):
    print("pre run strategy")
    # context.add_account(source, "15011218", 100000000.0)
    context.add_account(source, "15040900", 100000000.0)
    context.subscribe(source, ["601988", "600000"], exchange)
    context.add_timer(context.now() + kft.NANO_PER_SECOND, test_timer)
    context.add_time_interval(5 * kft.NANO_PER_SECOND, test_time_interval)
    # context.register_nanotime_callback(context.get_nano() + int(5* 1e9), insert_order)
    # sub_portfolio = context.get_sub_portfolio_info("15040900")
    # print(sub_portfolio)
    # portfolio = context.get_portfolio_info()
    # print(portfolio)

def insert_order(context, nano):

    order_id = context.insert_market_order("601988", exchange, "15040900", 200, Side.Buy, Offset.Open)
    context.log.info("insert market order: [order_id] %d [account] %s", order_id, "15040900")
    order_id = context.insert_limit_order("600000", exchange, "15011218", 12.8, 300, Side.Buy, Offset.Open)
    context.log.info("insert limit order: [order_id] %d [account] %s", order_id, "15011218")

def on_quote(context, quote):
    context.log.info("{} {}".format(quote.instrument_id, quote.trading_day))
    order_id = context.insert_market_order("601988", exchange, "15040900", 200, Side.Buy, Offset.Open)
    context.log.info("insert market order: [order_id] %d [account] %s", order_id, "15040900")
    pass

def on_transaction(context, transaction):
    context.log.info("{} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log.info("{} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.info('ticker {} order_id {}, status {}'.format(order.instrument_id, order.order_id, order.status))

def on_trade(context, trade):
    context.log.info('order_id {}'.format(trade.order_id))

def pre_quit(context):
    context.log.info("pre quit")
