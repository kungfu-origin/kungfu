import pyyjj
import pywingchun
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun.strategy import Strategy
from kungfu.log import create_logger


@kfc.command()
@click.option('-g', '--group', type=str, default='default', help='group')
@click.option('-n', '--name', type=str, required=True, help='name')
@click.option('-p', '--path', type=str, required=True, help='path of strategy py file')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def strategy(ctx, group, name, path, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.path = path
    ctx.low_latency = low_latency
    ctx.logger = create_logger(name, ctx.log_level, pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.STRATEGY, group, name, ctx.locator))
    strategy = Strategy(ctx)
    runner = pywingchun.Runner(ctx.low_latency, ctx.locator, ctx.group, ctx.name)
    runner.add_strategy(strategy)
    runner.run()
