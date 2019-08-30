import pyyjj
import json
import click
from test import test
from test import pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger
from kungfu.wingchun.constants import Source, Exchange
from kungfu.wingchun.strategy import Strategy
from kungfu.wingchun import Runner, replay_setup
from mock import patch

@test.group()
@click.option('-g', '--group', type=str, default='default', help='group')
@click.option('-n', '--name', type=str, required=True, help='tester name')
@click.pass_context
def strategy(ctx, group, name):
    pass_ctx_from_root(ctx)
    ctx.group = group
    ctx.name = name
    ctx.category = 'strategy'
    ctx.mode = "live"
    ctx.low_latency = False
    ctx.logger = create_logger(ctx.name, ctx.log_level, pyyjj.location(pyyjj.get_mode_by_name(ctx.mode), pyyjj.category.STRATEGY, ctx.group, ctx.name, ctx.locator))
    ctx.path = None
    ctx.runner = Runner(ctx, pyyjj.get_mode_by_name(ctx.mode))
    def init_strategy_patch(self, path):
       pass
    with patch.object(Strategy, '_Strategy__init_strategy', init_strategy_patch):
        ctx.strategy = Strategy(ctx)

def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.category = ctx.parent.category
    ctx.mode = ctx.parent.mode
    ctx.low_latency = ctx.parent.low_latency
    ctx.logger = ctx.parent.logger
    ctx.strategy = ctx.parent.strategy
    ctx.runner = ctx.parent.runner

@strategy.command()
@click.option('-s', '--source_name', type=str, required=True, help='source name')
@click.option('-t', '--ticker', type=str, help='ticker to subscribe')
@click.option('-e', '--exchange_id', type=str, help="exchange id")
@click.pass_context
def sub(ctx, source_name, ticker, exchange_id):
    pass_ctx_from_parent(ctx)
    ctx.strategy._pre_start = lambda ctx: ctx.subscribe(source_name, [ticker], exchange_id)
    ctx.strategy._post_start = lambda ctx: None
    ctx.strategy._on_trading_day = lambda ctx, daytime: None
    ctx.strategy._on_quote = lambda ctx, quote: ctx.logger.info("quote received: {}.{} {}|{} {}|{}".format(quote.instrument_id, quote.exchange_id, quote.bid_price[0], quote.bid_volume[0], quote.ask_price[0], quote.ask_volume[0]))
    ctx.runner.add_strategy(ctx.strategy)
    ctx.runner.run()

@strategy.group()
@click.pass_context
def order(ctx):
    pass_ctx_from_parent(ctx)

@order.group()
@click.pass_context
@click.option('-t', '--ticker', type=str, help='ticker to subscribe')
@click.option('-e', '--exchange_id', type=str, help="exchange id")
def new(ctx,ticker, exchange_id):
    pass_ctx_from_parent(ctx)
    print(ticker, exchange_id)

@order.group()
@click.pass_context
@click.option('-i', '--order_id', type=int, help='order to cancel')
def cancel(ctx, order_id):
    pass_ctx_from_parent(ctx)
