import click

from kungfu.command.history import history, pass_ctx_from_parent
from kungfu.converter import Converter

import kungfu.yijinjing.time as kft


@history.command()
@click.option('-f', '--filepath', type=str, help='csv_file_path')
@click.option('-t', '--time', type=str, help='time')
@click.pass_context
def importer(ctx, filepath, time):
    pass_ctx_from_parent(ctx)
    ctx.filepath = filepath
    ctx.time = time
    cv = Converter(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()