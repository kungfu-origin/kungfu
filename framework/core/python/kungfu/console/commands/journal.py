import click
import glob
import kungfu
import platform
import os
import shutil
import zipfile

from datetime import datetime, timedelta
from tabulate import tabulate

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing import time as kft
from kungfu.yijinjing.log import create_logger


SESSION_DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S"
FRAME_TIME_FORMAT = "%H:%M:%S.%N"
DURATION_FORMAT = "%H:%M:%S.%N"
DURATION_TZ_ADJUST = int(
    timedelta(hours=datetime.fromtimestamp(0).hour).total_seconds() * 1e9
)

yjj = kungfu.__bindings__.yijinjing


@kfc.group(cls=PrioritizedCommandGroup)
@click.option(
    "-m", "--mode", default="*", type=click.Choice(kfj.MODES.keys()), help="mode"
)
@click.option(
    "-c",
    "--category",
    default="*",
    type=click.Choice(kfj.CATEGORIES.keys()),
    help="category",
)
@click.option("-g", "--group", type=str, default="*", help="group")
@click.option("-n", "--name", type=str, default="*", help="name")
@click.help_option("-h", "--help")
@click.pass_context
def journal(ctx, mode, category, group, name):
    kfc.pass_ctx_from_parent(ctx)
    ctx.low_latency = False
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.location = yjj.location(
        kfj.MODES[mode], kfj.CATEGORIES[category], group, name, ctx.runtime_locator
    )
    ctx.logger = create_logger("journal", ctx.log_level, ctx.console_location)

    (ctx.console_width, ctx.console_height) = shutil.get_terminal_size((0, 0))
    yjj.setup_log(ctx.console_location, "journal")


def pass_ctx_from_parent(ctx):
    kfc.pass_ctx_from_parent(ctx)
    ctx.logger = ctx.parent.logger
    ctx.low_latency = ctx.parent.low_latency
    ctx.mode = ctx.parent.mode
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.runtime_dir = ctx.parent.runtime_dir
    ctx.archive_dir = ctx.parent.archive_dir
    ctx.dataset_dir = ctx.parent.dataset_dir
    ctx.inbox_dir = ctx.parent.inbox_dir
    ctx.runtime_locator = ctx.parent.runtime_locator
    ctx.config_location = ctx.parent.config_location
    ctx.console_location = ctx.parent.console_location
    ctx.index_location = ctx.parent.index_location
    ctx.location = ctx.parent.location
    ctx.console_width = ctx.parent.console_width
    ctx.console_height = ctx.parent.console_height


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


@journal.command()
@click.option("-i", "--session_id", type=int, required=True, help="session id")
@click.option(
    "-t",
    "--io_type",
    type=click.Choice(["all", "in", "out"]),
    default="all",
    help="input or output during this session",
)
@click.option(
    "-f",
    "--tablefmt",
    default="simple",
    type=click.Choice(
        ["plain", "simple", "orgtbl", "grid", "fancy_grid", "rst", "textile"]
    ),
    help="output format",
)
@click.option("-p", "--pager", is_flag=True, help="show in a pager")
@click.pass_context
def show(ctx, session_id, io_type, tablefmt, pager):
    pass_ctx_from_parent(ctx)
    journal_df = kfj.show_journal(ctx, session_id, io_type)
    journal_df["gen_time"] = journal_df["gen_time"].apply(
        lambda t: kft.strftime(t, FRAME_TIME_FORMAT)
    )
    journal_df["trigger_time"] = journal_df["trigger_time"].apply(
        lambda t: kft.strftime(t, FRAME_TIME_FORMAT)
    )

    table = tabulate(journal_df.values, headers=journal_df.columns, tablefmt=tablefmt)

    if pager:
        click.echo_via_pager(table)
    else:
        click.echo(table)


@journal.command()
@click.option("-i", "--session_id", type=int, required=True, help="session id")
@click.option(
    "-t",
    "--io_type",
    type=click.Choice(["all", "in", "out"]),
    default="all",
    help="input or output during this session",
)
@click.pass_context
def trace(ctx, session_id, io_type):
    pass_ctx_from_parent(ctx)
    kfj.trace_journal(ctx, session_id, io_type)


@journal.command()
@click.option("-A", "--archive", is_flag=True, help="archive")
@click.option("-D", "--dry", is_flag=True, help="dry run")
@click.pass_context
def clean(ctx, archive, dry):
    pass_ctx_from_parent(ctx)
    search_path = os.path.join(
        ctx.runtime_dir, "*", "*", "*", "journal", "*", "*.journal"
    )
    journal_files = glob.glob(search_path)
    if dry:
        for journal_file in journal_files:
            click.echo(f"rm {journal_file}")
        return
    if archive:
        datestr = kft.strfnow("%Y%m%d-%H%M%S")
        archive_path = os.path.join(ctx.home, f"KFA-{datestr}.zip")
        archive_zip = zipfile.ZipFile(
            archive_path, mode="w", compression=zipfile.ZIP_LZMA
        )
        for journal_file in journal_files:
            archive_zip.write(journal_file)
        click.echo(f"archived to {archive_path}")
    for journal_file in journal_files:
        os.remove(journal_file)
    click.echo(f"cleaned {len(journal_files)} journal files")
