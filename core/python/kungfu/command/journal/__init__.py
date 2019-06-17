import pyyjj
import kungfu.yijinjing.journal as kfj
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root


@kfc.group()
@click.option('-m', '--mode', default='*', type=click.Choice(kfj.MODES.keys()), help='mode')
@click.option('-c', '--category', default='*', type=click.Choice(kfj.CATEGORIES.keys()), help='category')
@click.option('-g', '--group', type=str, default='*', help='group')
@click.option('-n', '--name', type=str, default='*', help='name')
@click.pass_context
def journal(ctx, mode, category, group, name):
    pass_ctx_from_root(ctx)
    ctx.low_latency = False
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.location = pyyjj.location(kfj.MODES[mode], kfj.CATEGORIES[category], group, name, ctx.locator)
    pyyjj.setup_log(ctx.location, 'journal_tools')


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.low_latency = ctx.parent.low_latency
    ctx.mode = ctx.parent.mode
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.location = ctx.parent.locator
