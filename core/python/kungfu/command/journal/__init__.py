import pyyjj
import click
from kungfu.command import kfc


@kfc.group()
@click.pass_context
def journal(ctx):
    pyyjj.setup_log('journal_tools')
