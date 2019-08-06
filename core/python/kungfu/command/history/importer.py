import click
import os, pkgutil
from kungfu.command.history import history, pass_ctx_from_parent
#from kungfu.converter import Converter
from extensions import EXTENSION_REGISTRY_BT
import kungfu.yijinjing.time as kft


@history.command()
@click.option('-s', '--source', type=str, help='select data source')
@click.option('-a', '--arguments', nargs=2, type=str, multiple=True, help='argument format: <arg_name> <arg_value>')
@click.pass_context
def importer(ctx, source, arguments):
    pass_ctx_from_parent(ctx)
    ctx.arguments = {arg[0]: arg[1] for arg in arguments}
    cv = EXTENSION_REGISTRY_BT.get_extension(source)(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()

# yarn dev history -g xtp -n xtp importer -s csvConverter -a file_path /Users/yetaoran/Documents/futures/2016/201601/20160104/Daily_20160104_AL1601.csv -a time 0