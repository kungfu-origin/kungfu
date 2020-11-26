'''
@Author: your name
@Date: 2020-06-10 13:39:46
@LastEditTime: 2020-06-10 17:57:44
@LastEditors: your name
@Description: In User Settings Edit
@FilePath: /ucbet/Users/zhangyizhi/Project/kungfu/examples/strategy/py/xtp_demo.py
'''
import random
from kungfu.wingchun.constants import *
import pandas

def test_timer(context, event):
    context.log.info('test timer')


def test_time_interval(context, event):
    context.log.info('test time interval')
    context.log.info('test time interval')


def pre_start(context):
    # context.hold_book()
    # context.hold_positions()
    context.log.info(f"is_book_held: {context.is_book_held()}, is_positions_mirrored: {context.is_positions_mirrored()}")
    context.log.info("pre start")
    context.add_account(Source.XTP, "15040900", 100000000.0)
    context.subscribe(Source.XTP, ["600198", "600548"], Exchange.SSE)
    # context.subscribe(source, ["159901", "300030"], Exchange.SZE)
    # context.subscribe(Source.BAR, ["159901", "300030"], Exchange.SZE)


def post_start(context):
    context.log.info('strategy post start')


def pre_stop(context):
    context.log.info('strategy going down')


def post_stop(context):
    context.log.info('strategy down')


def on_bar(context, bar):
    context.log.info("[on_bar] {}".format(bar))


def on_quote(context, quote):
    # context.logger.info(f"quote: {quote}")
    side = random.choice([Side.Buy, Side.Sell])
    side = Side.Buy
    price = quote.ask_price[0] if side == Side.Buy else quote.bid_price[0]
    price_type = random.choice([PriceType.Any, PriceType.Limit])
    context.insert_order(quote.instrument_id, Exchange.SSE, "15040900", price, 100, price_type, side)


def on_transaction(context, transaction):
    pass


def on_entrust(context, entrust):
    pass


def on_order(context, order):
    if order.error_id != 0:
        context.log.info(f'order error {order.error_msg}')


def on_trade(context, trade):
    pass
