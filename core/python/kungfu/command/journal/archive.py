import click
import os
import shutil
import kungfu.yijinjing.journal as kfj
from kungfu.command.journal import journal, pass_ctx_from_parent

from pykungfu import yijinjing as yjj


@journal.command()
@click.pass_context
def archive(ctx):
    pass_ctx_from_parent(ctx)
    archive_locator = kfj.Locator(ctx.archive_dir)
    assemble = yjj.assemble([ctx.runtime_locator])
    assemble >> yjj.fixed_sink(archive_locator)

    index_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'journal', 'index', archive_locator)
    io_device = yjj.io_device(index_location, True, True)
    session_builder = yjj.session_builder(io_device)
    session_builder.rebuild_index_db()

    # shutil.make_archive("yjj_archive", "zip", target_dir)
