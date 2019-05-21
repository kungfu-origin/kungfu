
from kungfu.wingchun.constants import *
source = Source.XTP
exchange = Exchange.SSE

def init(context):
    context.add_md(source)
    context.add_account(source, "15011218", 100000000.0)
    context.add_account(source, "15040900", 100000000.0)
    context.subscribe(source, ["601988", "600000"], exchange, True)
    context.register_nanotime_callback_with_context(context.get_nano() + 5* 1e9, insert_order)
    sub_portfolio = context.get_sub_portfolio_info("15040900")
    print(sub_portfolio)
    portfolio = context.get_portfolio_info()
    print(portfolio.trading_day, type(portfolio.trading_day))
    print(portfolio.client_id, type(portfolio.client_id))
    print(portfolio)

def pre_run(context):
    pass


def insert_order(context, nano):

    order_id = context.insert_market_order("601988", exchange, "15040900", 200, Side.Buy, Offset.Open)
    context.log_info("insert market order: [order_id] {} [account] {}".format(order_id, "15040900"))
    order_id = context.insert_limit_order("600000", exchange, "15011218", 12.8, 300, Side.Buy, Offset.Open)
    context.log_info("insert limit order: [order_id] {} [account] {}".format(order_id, "15011218"))

def on_transaction(context, transaction):
    context.log_info("[on_transaction] {} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log_info("[on_entrust] {} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log_info('[on_order] rcv_time {} ticker {} order_id {}, status {}|{}'.format(order.rcv_time, order.instrument_id, order.order_id, order.status, OrderStatus.to_str(order.status)))

def on_trade(context, trade):
    context.log_info('[on_trade] rcv_time {} order_id {}'.format(trade.rcv_time, trade.order_id))

def pre_quit(context):
    context.log_info("pre quit")
