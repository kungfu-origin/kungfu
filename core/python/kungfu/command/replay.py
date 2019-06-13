import click
from kungfu.command import kfc
from kungfu.wingchun.strategy import Strategy
from kungfu.practice.apprentice import Apprentice, EventHandler


@kfc.command()
@click.option('-p', '--path', type=str, help='path of strategy py file')
@click.pass_context
def replay(ctx, path):
    ctx.logger.info('strategy command try')
    strategy = Strategy(ctx.logger, ctx.name, path)
    handler = EventHandler(ctx.logger, strategy)
    apprentice = Apprentice(ctx, ctx.logger)
    apprentice.add_event_handler(handler)
    apprentice.go()