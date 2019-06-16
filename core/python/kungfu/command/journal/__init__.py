import pyyjj
import kungfu.yijinjing.journal as kfj
import click
from kungfu.command import kfc, pass_ctx_from_parent


@kfc.group()
@click.option('-m', '--mode', default='*', type=click.Choice(kfj.MODES.keys()), help='mode')
@click.option('-c', '--category', default='*', type=click.Choice(kfj.CATEGORIES.keys()), help='category')
@click.option('-g', '--group', type=str, default='*', help='group')
@click.option('-n', '--name', type=str, default='*', help='name')
@click.pass_context
def journal(ctx, mode, category, group, name):
    pass_ctx_from_parent(ctx)
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.location = pyyjj.location(kfj.MODES[mode], kfj.CATEGORIES[category], group, name, ctx.locator)
    pyyjj.setup_log(ctx.location, 'journal_tools')
