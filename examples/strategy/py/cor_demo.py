from kungfu.wingchun.constants import *


def pre_start(context):
    context.add_account(Source.XTP, "15014990", 100000000.0)
    context.subscribe(Source.XTP, ["600198"], Exchange.SSE)


async def on_quote(context, quote, location):
    context.log.info(f'on quote {quote.instrument_id}')
    order = await context.buy(quote.instrument_id, Exchange.SSE, "15014990", quote.ask_price[0], 100)
    context.log.info(f'buy order status {order.status}')
    order = await context.sell(quote.instrument_id, Exchange.SSE, "15014990", quote.bid_price[0], 100)
    context.log.info(f'sell order status {order.status}')
