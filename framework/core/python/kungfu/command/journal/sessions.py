import click
import kungfu
import platform
import shutil

from tabulate import tabulate
from kungfu.yijinjing import time as kft
from kungfu.yijinjing import journal as kfj
from kungfu.command.journal import journal, pass_ctx_from_parent

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing


@journal.command()
@click.option(
    "-s",
    "--sortby",
    default="begin_time",
    type=click.Choice(
        ["begin_time", "end_time", "duration", "mode", "category", "group", "name"]
    ),
    help="sorting method",
)
@click.option("-a", "--ascending", is_flag=True, help="sorted as ascending")
@click.option(
    "-f",
    "--tablefmt",
    default="simple",
    type=click.Choice(
        ["plain", "simple", "orgtbl", "grid", "fancy_grid", "rst", "textile"]
    ),
    help="output format",
)
@click.pass_context
def sessions(ctx, sortby, ascending, tablefmt):
    pass_ctx_from_parent(ctx)
    all_sessions = kfj.find_sessions(ctx).sort_values(by=sortby, ascending=ascending)
    all_sessions["begin_time"] = all_sessions["begin_time"].apply(
        lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT)
    )
    all_sessions["end_time"] = all_sessions["end_time"].apply(
        lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT)
    )
    all_sessions["duration"] = all_sessions["duration"].apply(
        lambda t: kft.strftime(t - kft.DURATION_TZ_ADJUST, kft.DURATION_FORMAT)
    )

    table = tabulate(
        all_sessions.values, headers=all_sessions.columns, tablefmt=tablefmt
    )

    (term_width, term_height) = shutil.get_terminal_size()
    if term_height < len(all_sessions) + 2 and platform.system() != "Windows":
        click.echo_via_pager(table)
    else:
        click.echo(table)


@journal.command()
@click.pass_context
def rebuild_index(ctx):
    pass_ctx_from_parent(ctx)
    io_device = yjj.io_device(ctx.console_location)
    session_builder = yjj.session_builder(io_device)
    click.echo("rebuild sqlite db")
    session_builder.rebuild_index_db()
    click.echo("done")
