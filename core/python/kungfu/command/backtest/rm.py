import click
import os, glob, shutil

from kungfu.command.backtest import backtest, pass_ctx_from_parent
import kungfu.yijinjing.time as kft

@backtest.command()
@click.option('-f', '--filepath', type=str, help='csv_file_path')
@click.option('-t', '--time', type=str, help='time')
@click.pass_context
def rm(ctx, filepath, time):
    pass_ctx_from_parent(ctx)

    journal_path = os.path.join(ctx.home, ctx.category, ctx.group, ctx.name)
    if os.path.exists(journal_path) or '*' in journal_path:
        for file in glob.glob(journal_path):
            shutil.rmtree(file)
            click.echo('history journals in {} is cleared'.format(file))
    else:
        click.echo('invalid journal path')

# yarn dev backtest -g tushare rm