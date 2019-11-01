
import click
from tabulate import tabulate
from kungfu.command import kfc
from kungfu.msg import Registry

@kfc.group()
@click.pass_context
def msg(ctx):
    pass

@msg.command(help='list msg types')
@click.option('-f', '--tablefmt', default='simple',
              type=click.Choice(['plain', 'simple', 'orgtbl', 'grid', 'fancy_grid', 'rst', 'textile']),
              help='output format')
@click.option('-p', '--pager', is_flag=True, help='show in a pager')
@click.pass_context
def list(ctx, tablefmt, pager):
    types = Registry.types()
    if types:
        table = tabulate([t.values() for t in types], headers=types[0].keys(), tablefmt=tablefmt)
        if pager:
            click.echo_via_pager(table)
        else:
            click.echo(table)
    else:
        click.echo("no msg type registered")