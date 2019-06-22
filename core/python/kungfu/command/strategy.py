import click
from kungfu.command import kfc
from kungfu.wingchun.strategy import Strategy


@kfc.command()
@click.option('-p', '--path', type=str, help='path of strategy py file')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def strategy(ctx, path, low_latency):
    ctx.parent.low_latency = low_latency
    strategy = Strategy(ctx.parent, path)
