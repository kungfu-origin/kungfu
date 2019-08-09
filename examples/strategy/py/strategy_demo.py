import sys
import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

source = Source.XTP
exchange = Exchange.SSE

def test_timer(context, event):
    context.log.info('test timer')

def test_time_interval(context, event):
    context.log.info('test time interval')

def pre_start(context):
    context.log.info("pre run strategy")
    #context.add_account(source, "15011218", 100000000.0)
    context.add_account(source, "15040900", 100000000.0)
    context.subscribe(source, ["600000", "601988"], exchange)
    #context.add_timer(context.now() + kft.NANO_PER_SECOND, test_timer)
    #context.add_time_interval(5 * kft.NANO_PER_SECOND, test_time_interval)
    # context.register_nanotime_callback(context.get_nano() + int(5* 1e9), insert_order)
    # sub_portfolio = context.get_sub_portfolio_info("15040900")
    # print(sub_portfolio)
    # portfolio = context.get_portfolio_info()
    # print(portfolio)

# def insert_order(context, nano):
#     order_id = context.insert_market_order("601988", exchange, "15040900", 200, Side.Buy, Offset.Open)
#     context.log.info("insert market order: [order_id] %d [account] %s", order_id, "15040900")
#     order_id = context.insert_limit_order("600000", exchange, "15011218", 12.8, 300, Side.Buy, Offset.Open)
#     context.log.info("insert limit order: [order_id] %d [account] %s", order_id, "15011218")

def on_quote(context, quote):
    #context.log.info("{} {}".format(quote.data_time, quote.last_price))
    order_id = context.insert_limit_order("601988", exchange, "15040900", 75, 200, Side.Buy, Offset.Open)
    context.log.info("quote received: [time]{} [instrument_id]{} [last_price]{}".format(kft.strftime(quote.data_time), quote.instrument_id, quote.last_price))
    # context.insert_market_order("000001.SZ", "", "15040900", 20, Side.Buy, Offset.Open)
    pass

def on_transaction(context, transaction):
    context.log.info("{} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log.info("{} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.info('order received: [instrument_id]{} [volume]{} [price]{}'.format(order.instrument_id, order.volume, order.limit_price))
    pass

def on_trade(context, trade):
    context.log.info('trade received: [trade_id]{} [volume]{} [price]{}'.format(trade.order_id, trade.volume, trade.price))
    pass

def pre_stop(context):
    context.log.info("pre stop")
    print(context.get_quotes()[0])
    print(context.get_orders()[0])
    print(context.get_trades()[0])
    # 可根据quotes orders trades 的记录计算各类统计数据 TODO

# yarn dev -l trace strategy -b -g tsConverter -n 20190808 -p ../examples/strategy/py/strategy_demo.py -i 1