import click
import os
import shutil
from kungfu.command.journal import journal, pass_ctx_from_parent
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.sinks import ArchiveSink

from pykungfu import yijinjing as yjj


@journal.command()
@click.option('-f', '--format', type=click.Choice(['zip', 'tar']), default='zip', help='archive format')
@click.pass_context
def archive(ctx, format):
    pass_ctx_from_parent(ctx)
    ctx.logger = create_logger('archive', ctx.log_level, ctx.assemble_location)
    assemble = yjj.assemble([ctx.runtime_locator])
    assemble >> ArchiveSink(ctx)

    os.chdir(ctx.archive_dir)
    for archive_date in filter(lambda path : os.path.isdir(path), sorted(os.listdir(os.curdir))):
        ctx.logger.info(f'compressing archive for {archive_date}')
        archive_locator = kfj.Locator(archive_date)
        index_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'journal', 'index', archive_locator)
        io_device = yjj.io_device(index_location, True, True)
        session_builder = yjj.session_builder(io_device)
        session_builder.rebuild_index_db()
        archive_name = 'KFA-' + archive_date
        archive_file = f'{archive_name}.{format}'
        if os.path.exists(archive_file):
            ctx.logger.warn(f'removed duplicated {archive_file}')
            os.remove(archive_file)
        shutil.make_archive(archive_name, format, archive_date)
        shutil.rmtree(archive_date)
