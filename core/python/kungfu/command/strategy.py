import click
from kungfu.command import kfc
from kungfu.wingchun.strategy import Strategy
from kungfu.practice.apprentice import Apprentice, EventHandler


@kfc.command()
@click.option('-p', '--path', type=str, help='path of strategy py file')
@click.pass_context
def strategy(ctx, path):
    handler = EventHandler(ctx.parent, Strategy(ctx.parent, path))
    apprentice = Apprentice(ctx.parent)
    apprentice.add_event_handler(handler)
    apprentice.go()
