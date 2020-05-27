import click
import functools
import os
import shutil
from collections import deque
from kungfu.command.journal import journal, pass_ctx_from_parent
from kungfu.yijinjing import ARCHIVE_PREFIX
from kungfu.yijinjing.locator import Locator
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.journal import prune_journals
from kungfu.yijinjing.sinks import ArchiveSink

from pykungfu import yijinjing as yjj


@journal.command()
@click.option('-f', '--format', type=click.Choice(['zip', 'tar']), default='zip', help='archive format')
@click.pass_context
def archive(ctx, format):
    pass_ctx_from_parent(ctx)
    ctx.logger = create_logger('archive', ctx.log_level, ctx.assemble_location)

    os.chdir(ctx.archive_dir)

    ctx.logger.info('cleaning up archive folder')
    deque(map(shutil.rmtree, filter(os.path.isdir, os.listdir(os.curdir))), maxlen=0)

    ctx.logger.info('exporting archive journals')
    assemble = yjj.assemble([ctx.runtime_locator])
    assemble >> ArchiveSink(ctx)

    ctx.logger.info('compressing archive files')
    deque(map(functools.partial(make, ctx, format), filter(os.path.isdir, sorted(os.listdir(os.curdir)))), maxlen=0)

    ctx.logger.info('pruning runtime journals')
    prune_journals(ctx, ctx.runtime_dir)

    ctx.logger.info('archive done')


def make(ctx, archive_format, archive_date):
    archive_locator = Locator(archive_date)
    index_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'journal', 'index', archive_locator)
    io_device = yjj.io_device(index_location, True, True)
    session_builder = yjj.session_builder(io_device)
    session_builder.rebuild_index_db()
    archive_name = f'{ARCHIVE_PREFIX}-{archive_date}'
    archive_file = f'{archive_name}.{archive_format}'
    if os.path.exists(archive_file):
        ctx.logger.warn(f'removed duplicated {archive_file}')
        os.remove(archive_file)
    shutil.make_archive(archive_name, archive_format, archive_date)
    shutil.rmtree(archive_date)
    ctx.logger.info(f'compressed archive for {archive_date}')
