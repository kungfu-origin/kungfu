import pyyjj
import kungfu.yijinjing.journal as kfj
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger


@kfc.group(invoke_without_command=True)
@click.option('-m', '--mode', default='*', type=click.Choice(kfj.MODES.keys()), help='mode')
@click.option('-c', '--category', default='*', type=click.Choice(kfj.CATEGORIES.keys()), help='category')
@click.option('-g', '--group', type=str, default='*', help='group')
@click.option('-n', '--name', type=str, default='*', help='name')
@click.pass_context
def history(ctx, mode, category, group, name):
    pass_ctx_from_root(ctx)
    ctx.low_latency = False
    ctx.mode = 'backtest'
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.location = pyyjj.location(kfj.MODES[ctx.mode], kfj.CATEGORIES[category], group, name, ctx.locator)
    ctx.journal_util_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'util', 'journal', ctx.locator)
    ctx.logger = create_logger('journal', ctx.log_level, ctx.journal_util_location)
    #pyyjj.setup_log(ctx.journal_util_location, 'journal')

    if help:
        click.echo(converter.get_help(ctx))
        return

    pass_ctx_from_parent(ctx)
    if clear:
        journal_path = os.path.join(ctx.home, 'md', group, name, 'journal', 'backtest')
        if os.path.exists(journal_path):
            for file in glob.glob(journal_path + os.sep + '*'):
                os.remove(file)
            click.echo('history journals in {} is cleared'.format(journal_path))
        else:
            click.echo('invalid journal path')
        return

    ctx.grp = group
    ctx.nm = name
    ctx.filepath = filepath
    ctx.time = time
    cv = Converter(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()

def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
    ctx.low_latency = ctx.parent.low_latency
    ctx.mode = ctx.parent.mode
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.locator = ctx.parent.locator
    ctx.location = ctx.parent.location
    ctx.journal_util_location = ctx.parent.journal_util_location

def pass_ctx_from_parent(ctx):
    ctx.home = ctx.parent.home
    ctx.log_level = ctx.parent.log_level
    ctx.locator = ctx.parent.locator
    ctx.system_config_location = ctx.parent.system_config_location
    ctx.name = ctx.parent.name