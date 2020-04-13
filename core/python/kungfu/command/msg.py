import click
from pykungfu import longfist
from tabulate import tabulate
from kungfu.command import kfc


@kfc.group()
@click.pass_context
def msg(ctx):
    pass


@msg.command(help='list msg types')
def list():
    types = [str(t) for t in filter(lambda t: not t.startswith('_'), dir(longfist.types))]
    table = tabulate(types, tablefmt='simple')
    click.echo(table)
