import click
import os
import datetime

from kungfu.command.backtest import backtest, pass_ctx_from_parent
from extensions import EXTENSION_REGISTRY_BT

@backtest.command()
@click.option('-a', '--arguments', nargs=2, type=str, multiple=True, help='argument format: <arg_name> <arg_value>')
@click.pass_context
def add(ctx, arguments):
    pass_ctx_from_parent(ctx)
    if ctx.name == '*':
        ctx.name = datetime.datetime.today().strftime('%Y%m%d')
    ctx.arguments = {arg[0]: arg[1] for arg in arguments}
    cv = EXTENSION_REGISTRY_BT.get_extension(ctx.group)(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()