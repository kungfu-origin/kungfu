import pyyjj
import shutil
from tabulate import tabulate

import click
from kungfu.command.journal import journal, pass_ctx_from_parent

import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj


@journal.command()
@click.option('-s', '--sortby', default='begin_time',
              type=click.Choice(['begin_time', 'end_time', 'duration', 'mode', 'category', 'group', 'name']),
              help='sorting method')
@click.option('-a', '--ascending', is_flag=True, help='sorted as ascending')
@click.option('-f', '--tablefmt', default='simple',
              type=click.Choice(['plain', 'simple', 'orgtbl', 'grid', 'fancy_grid', 'rst', 'textile']),
              help='output format')
@click.pass_context
def sessions(ctx, sortby, ascending, tablefmt):
    pass_ctx_from_parent(ctx)
    all_sessions = kfj.find_sessions(ctx).sort_values(by=sortby, ascending=ascending)
    all_sessions['begin_time'] = all_sessions['begin_time'].apply(lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT))
    all_sessions['end_time'] = all_sessions['end_time'].apply(lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT))
    all_sessions['duration'] = all_sessions['duration'].apply(lambda t: kft.strftime(t - kft.DURATION_TZ_ADJUST, kft.DURATION_FORMAT))

    table = tabulate(all_sessions.values, headers=all_sessions.columns, tablefmt=tablefmt)

    (term_width, term_height) = shutil.get_terminal_size()
    if term_height < len(all_sessions) + 2:
        click.echo_via_pager(table)
    else:
        click.echo(table)


@journal.command()
@click.pass_context
def rebuild_index(ctx):
    pass_ctx_from_parent(ctx)
    io_device = pyyjj.io_device_master(ctx.journal_util_location, False)
    click.echo("rebuild sqlite db")
    io_device.rebuild_index_db()
    click.echo("done")


@journal.command()
@click.pass_context
def sql(ctx):
    pass_ctx_from_parent(ctx)
    io_device = pyyjj.io_device(ctx.journal_util_location)
    click.echo("read from sqlite")
    res = io_device.find_sessions()
    click.echo(res)
