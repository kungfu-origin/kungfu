import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.log import create_logger
from kungfu.practice.master import Master


@kfc.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def master(ctx, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.logger = create_logger("master", ctx.log_level, pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "master", "master", ctx.locator))
    ctx.low_latency = low_latency
    server = Master(ctx)
    server.run()

