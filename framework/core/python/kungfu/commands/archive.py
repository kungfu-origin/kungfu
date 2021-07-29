import click
import functools
import glob
import kungfu
import os
import shutil

from collections import deque

from kungfu.commands import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.commands import PrioritizedCommandGroup
from kungfu.yijinjing import LOG_PATTERN, ARCHIVE_PREFIX
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.locator import Locator
from kungfu.yijinjing.sinks.archive import ArchiveSink
from kungfu.yijinjing.utils import prune_layout_files

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing


@kfc.group(cls=PrioritizedCommandGroup)
@click.help_option("-h", "--help")
@click.pass_context
def archive(ctx):
    pass_ctx_from_root(ctx)
    ctx.low_latency = False
    ctx.logger = create_logger("archive", ctx.log_level, ctx.console_location)
    yjj.setup_log(ctx.console_location, "archive")


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
    ctx.low_latency = ctx.parent.low_latency
    ctx.runtime_dir = ctx.parent.runtime_dir
    ctx.archive_dir = ctx.parent.archive_dir
    ctx.dataset_dir = ctx.parent.dataset_dir
    ctx.inbox_dir = ctx.parent.inbox_dir
    ctx.runtime_locator = ctx.parent.runtime_locator
    ctx.config_location = ctx.parent.config_location
    ctx.console_location = ctx.parent.console_location
    ctx.index_location = ctx.parent.index_location


@archive.command()
@click.option(
    "-f",
    "--format",
    type=click.Choice(["zip", "tar"]),
    default="zip",
    help="archive format",
)
@click.pass_context
def make(ctx, format):
    pass_ctx_from_parent(ctx)
    os.chdir(ctx.archive_dir)

    today_date = yjj.strftime(yjj.now_in_nano(), "%Y-%m-%d")
    today_archive_name = f"{ARCHIVE_PREFIX}-{today_date}.{format}"
    today_archive_path = os.path.join(ctx.archive_dir, today_archive_name)
    today_temp_path = os.path.join(ctx.archive_dir, ".today")

    ctx.logger.info("preparing archive folder")
    deque(map(shutil.rmtree, filter(os.path.isdir, os.listdir(os.curdir))))

    if os.path.exists(today_archive_path):
        shutil.unpack_archive(today_archive_path, today_temp_path)
        export_logs(ctx, today_temp_path, ctx.archive_dir)
    else:
        os.makedirs(today_temp_path)

    ctx.logger.info("exporting journals")
    yjj.assemble([ctx.runtime_locator, Locator(today_temp_path)]) >> ArchiveSink(ctx)
    shutil.rmtree(today_temp_path)

    ctx.logger.info("exporting logs")
    export_logs(ctx, ctx.runtime_dir, ctx.archive_dir)

    ctx.logger.info("compressing archive files")
    deque(
        map(
            functools.partial(make_archive, ctx, format),
            filter(os.path.isdir, sorted(os.listdir(os.curdir))),
        )
    )

    ctx.logger.info("pruning runtime logs")
    prune_layout_files(ctx.runtime_dir, "log", "live")
    ctx.logger.info("pruning runtime journals")
    prune_layout_files(ctx.runtime_dir, "journal", "live")

    if os.path.exists(today_archive_path):
        shutil.unpack_archive(today_archive_path, ctx.runtime_dir)
    ctx.logger.info("archive done")


@archive.command()
@click.pass_context
def list(ctx):
    pass_ctx_from_parent(ctx)
    deque(map(print_archive, glob.glob(os.path.join(ctx.archive_dir, "*.zip"))))


def export_logs(ctx, src_dir, dst_dir):
    search_path = os.path.join(src_dir, "*", "*", "*", "log", "live", "*.log")
    for log_file in glob.glob(search_path):
        match = LOG_PATTERN.match(log_file[len(src_dir) + 1 :])
        if match:
            category = match.group(1)
            group = match.group(2)
            name = match.group(3)
            mode = match.group(4)
            date = match.group(6)
            archive_path = os.path.join(
                dst_dir, date, category, group, name, "log", mode
            )
            if not os.path.exists(archive_path):
                os.makedirs(archive_path)
            archive_log = os.path.join(archive_path, os.path.basename(log_file))
            if os.path.exists(archive_log):
                with open(log_file, "rb") as src, open(archive_log, "ab") as dst:
                    shutil.copyfileobj(src, dst)
            else:
                shutil.copy2(log_file, archive_path)
        else:
            ctx.logger.warn("unable to match log file %s", log_file)


def make_archive(ctx, archive_format, archive_date):
    archive_locator = Locator(archive_date)
    index_location = yjj.location(
        lf.enums.mode.LIVE,
        lf.enums.category.SYSTEM,
        "journal",
        "index",
        archive_locator,
    )
    io_device = yjj.io_device(index_location, True, True)
    session_builder = yjj.session_builder(io_device)
    session_builder.rebuild_index_db()
    archive_name = f"{ARCHIVE_PREFIX}-{archive_date}"
    archive_file = f"{archive_name}.{archive_format}"
    if os.path.exists(archive_file):
        ctx.logger.warn(f"removed duplicated {archive_file}")
        os.remove(archive_file)
    shutil.make_archive(archive_name, archive_format, archive_date)
    shutil.rmtree(archive_date)
    ctx.logger.info(f"compressed archive for {archive_date}")


def print_archive(archive_file):
    archive_name = os.path.basename(archive_file)
    print(archive_name[4:-4])
