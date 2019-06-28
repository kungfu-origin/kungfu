import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import Watcher


@kfc.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def watcher(ctx, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency
    Watcher(ctx).run()
