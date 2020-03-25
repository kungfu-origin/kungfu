from datetime import datetime, timedelta
from tabulate import tabulate

import click
from kungfu.command.journal import journal, pass_ctx_from_parent

import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj

SESSION_DATETIME_FORMAT = '%Y-%m-%d %H:%M:%S'
DURATION_FORMAT = '%H:%M:%S.%N'
DURATION_TZ_ADJUST = int(timedelta(hours=datetime.fromtimestamp(0).hour).total_seconds() * 1e9)


@journal.command()
@click.option('-i', '--session_id', type=int, required=True, help='session id')
@click.option('-t', '--io_type', type=click.Choice(['all', 'in', 'out']), default='all', help='input or output during this session')
@click.option('-f', '--tablefmt', default='simple',
              type=click.Choice(['plain', 'simple', 'orgtbl', 'grid', 'fancy_grid', 'rst', 'textile']),
              help='output format')
@click.option('-p', '--pager', is_flag=True, help='show in a pager')
@click.pass_context
def show(ctx, session_id, io_type, tablefmt, pager):
    pass_ctx_from_parent(ctx)
    journal_df = kfj.show_journal(ctx, session_id, io_type)
    journal_df['gen_time'] = journal_df['gen_time'].apply(lambda t: kft.strftime(t))
    journal_df['trigger_time'] = journal_df['trigger_time'].apply(lambda t: kft.strftime(t))

    table = tabulate(journal_df.values, headers=journal_df.columns, tablefmt=tablefmt)

    if pager:
        click.echo_via_pager(table)
    else:
        click.echo(table)

