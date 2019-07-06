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
@click.option('-p', '--pager', is_flag=True, help='show in a pager')
@click.pass_context
def info(ctx, session_id, pager):
    pass_ctx_from_parent(ctx)
    all_sessions = kfj.find_sessions(ctx)
    session = all_sessions[all_sessions['id'] == session_id]

    if pager:
        click.echo_via_pager(session)
    else:
        click.echo(session)

