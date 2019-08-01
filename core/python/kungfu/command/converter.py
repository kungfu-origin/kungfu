import click
import sys, os, glob
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.converter import Converter


@kfc.command()
@click.option('-h', '--help', is_flag=True, help='print help')
@click.option('-c', '--clear', is_flag=True, help='clear history journal')
@click.option('-g', '--group', type=str, help='group')
@click.option('-n', '--name', type=str, help='name')
@click.option('-f', '--filepath', type=str, help='csv_file_path')
@click.option('-t', '--time', type=str, help='time')
@click.pass_context
def converter(ctx, help, clear, group, name, filepath, time):
    if help:
        click.echo(converter.get_help(ctx))
        return

    pass_ctx_from_parent(ctx)
    if clear:
        journal_path = os.path.join(ctx.home, 'md', group, name, 'journal', 'backtest')
        if os.path.exists(journal_path):
            for file in glob.glob(journal_path + os.sep + '*'):
                os.remove(file)
            click.echo('history journals in {} is cleared'.format(journal_path))
        else:
            click.echo('invalid journal path')
        return

    ctx.grp = group
    ctx.nm = name
    ctx.filepath = filepath
    ctx.time = time
    cv = Converter(ctx)
    cv.write_data_to_journal()
    cv.read_data_from_journal()