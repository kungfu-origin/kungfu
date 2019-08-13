import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger
from extensions import EXTENSION_REGISTRY_BT


@kfc.group(help_priority=5)
@click.option('-g', '--group', type=click.Choice(EXTENSION_REGISTRY_BT.names()), required=True, help='group')
@click.option('-n', '--name', type=str, default='*', help='name')
@click.pass_context
def backtest(ctx, group, name):
    pass_ctx_from_root(ctx)
    ctx.category = 'md'
    ctx.group = group
    ctx.name = name
    ctx.mode = 'backtest'
    
    #ctx.location = pyyjj.location(kfj.MODES[ctx.mode], kfj.CATEGORIES[ctx.category], group, name, ctx.locator)
    ctx.journal_util_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'util', 'journal', ctx.locator)
    ctx.logger = create_logger('journal', ctx.log_level, ctx.journal_util_location)
    pyyjj.setup_log(ctx.journal_util_location, 'journal')


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
    ctx.mode = ctx.parent.mode
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    #ctx.location = ctx.parent.location
    ctx.journal_util_location = ctx.parent.journal_util_location