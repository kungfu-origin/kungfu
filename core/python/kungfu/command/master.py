import click
from kungfu.command import kfc
from kungfu.practice.master import Master


@kfc.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def master(ctx, low_latency):
    ctx.parent.low_latency = low_latency
    server = Master(ctx.parent)
    server.go()

