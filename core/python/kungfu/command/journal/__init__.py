import pyyjj
import kungfu.yijinjing.journal as journal
import click
from kungfu.command import kfc


@kfc.group()
@click.option('-m', '--mode', default='*', type=click.Choice(journal.MODES.keys()), help='mode')
@click.option('-c', '--category', default='*', type=click.Choice(journal.CATEGORIES.keys()), help='category')
@click.option('-g', '--group', type=str, default='*', help='group')
@click.option('-n', '--name', type=str, default='*', help='name')
@click.pass_context
def journal(ctx, mode, category, group, name):
    ctx.home = ctx.parent.home
    ctx.logger = ctx.parent.logger
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    pyyjj.setup_log('journal_tools')
