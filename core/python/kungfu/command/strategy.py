import click
from kungfu.command import kfc
from kungfu.wingchun.strategy import Strategy
from kungfu.practice.apprentice import Apprentice, EventHandler


@kfc.command()
@click.option('-p', '--path', type=str, help='path of strategy py file')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def strategy(ctx, path, low_latency):
    ctx.parent.low_latency = low_latency
    handler = EventHandler(ctx.parent, Strategy(ctx.parent, path))
    apprentice = Apprentice(ctx.parent)
    apprentice.add_event_handler(handler)
    apprentice.run()
