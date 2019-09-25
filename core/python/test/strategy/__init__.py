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
        ctx.strategy._pre_start = lambda ctx: None
        ctx.strategy._post_start = lambda ctx: None
        ctx.strategy._pre_stop = lambda ctx: None
        ctx.strategy._post_stop = lambda ctx: None
        ctx.strategy._on_trading_day = lambda ctx, daytime: None
        ctx.strategy._on_entrust = lambda ctx, entrust: None
        ctx.strategy._on_transaction = lambda ctx, transaction: None
        ctx.strategy._on_quote = lambda ctx, quote: ctx.logger.info("quote received: {}".format(quote))
        ctx.strategy._on_order = lambda ctx, order: ctx.logger.info("order received: {}".format(order))
        ctx.strategy._on_trade = lambda ctx, trade: ctx.logger.info("trade received: {}".format(trade))

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
@click.option('-s', '--source', type=str, required=True, help='source name')
@click.option('-t', '--ticker', type=str, help='ticker to subscribe')
@click.option('-e', '--exchange', type=str, help="exchange id")
@click.pass_context
def sub(ctx, source, ticker, exchange):
    pass_ctx_from_parent(ctx)
    ctx.strategy._pre_start = lambda ctx: ctx.subscribe(source, [ticker], exchange)
    ctx.runner.add_strategy(ctx.strategy)
    ctx.runner.run()

@strategy.group()
@click.pass_context
def order(ctx):
    pass_ctx_from_parent(ctx)

@order.command()
@click.pass_context
@click.option("-s", "--source", type=str, required=True, help='source name')
@click.option("-a", "--account", type=str, required=True, help='account id')
@click.option('-i', '--order-id', type=int, help='order to cancel')
def cancel(ctx, source, account, order_id):
    click.echo("cancel order {} for {}@{}".format(order_id, account, source))
    pass_ctx_from_parent(ctx)
    def pre_start(ctx):
        ctx.add_account(source, account, 1e7)
    def post_start(ctx):
        ctx.logger.info("cancel order {}".format(order_id))
        ctx.cancel_order(order_id)
    ctx.strategy._pre_start = pre_start
    ctx.strategy._post_start = post_start
    ctx.runner.add_strategy(ctx.strategy)
    ctx.runner.run()
