import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.converter import Converter


@kfc.command()
@click.option('-g', '--group', type=str, help='group')
@click.option('-n', '--name', type=str, help='name')
@click.option('-f', '--filepath', type=str, help='filepath')
@click.option('-t', '--time', type=str, help='time')
@click.pass_context
def converter(ctx, group, name, filepath, time):
    pass_ctx_from_parent(ctx)
    ctx.grp = group
    ctx.nm = name
    ctx.filepath = filepath
    ctx.time = time
    cv = Converter(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()