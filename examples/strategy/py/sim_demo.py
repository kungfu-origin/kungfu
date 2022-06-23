import random
from kungfu.wingchun.constants import *

source = Source.SIM
exchange = Exchange.SSE


def pre_start(context):
    context.log.info(f"is_book_held: {context.is_book_held()}, is_positions_mirrored: {context.is_positions_mirrored()}")
    context.log.info("pre start")
    context.add_account(source, "123456", 100000000.0)
    context.subscribe(source, ["600000"], exchange)


def on_quote(context, quote, location):
    side = random.choice([Side.Buy, Side.Sell])
    side = Side.Buy
    price = quote.ask_price[0] if side == Side.Buy else quote.bid_price[0]
    price_type = random.choice([PriceType.Any, PriceType.Limit])
    context.insert_order(quote.instrument_id, exchange, source, "123456", price, 100, price_type, side)


def on_order(context, order, location):
    if order.error_id != 0:
        context.log.info(f'order error {order.error_msg}')


def on_trade(context, trade, location):
    context.log.info(f'traded')