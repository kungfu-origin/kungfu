import click
import os, glob

from kungfu.command.history import history, pass_ctx_from_parent
import kungfu.yijinjing.time as kft

@history.command()
@click.option('-f', '--filepath', type=str, help='csv_file_path')
@click.option('-t', '--time', type=str, help='time')
@click.pass_context
def rm(ctx, filepath, time):
    pass_ctx_from_parent(ctx)

    journal_path = os.path.join(ctx.home, ctx.category, ctx.group, ctx.name, 'journal', ctx.mode)

    if os.path.exists(journal_path):
        for file in glob.glob(journal_path + os.sep + '*'):
            os.remove(file)
        click.echo('history journals in {} is cleared'.format(journal_path))
    else:
        click.echo('invalid journal path')

# yarn dev history -g xtp -n csvConverter rm