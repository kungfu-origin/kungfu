import pyyjj
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
    session = kfj.find_session(ctx, session_id)
    uname = '{}/{}/{}/{}'.format(session['category'], session['group'], session['name'], session['mode'])
    uid = pyyjj.hash_str_32(uname)
    ctx.category = '*'
    ctx.group = '*'
    ctx.name = '*'
    ctx.mode = '*'
    locations = kfj.collect_journal_locations(ctx)
    location = locations[uid]
    for dest in location['readers']:
        dest_id = int(dest, 16)
        if dest_id == 0:
            click.echo('{} -> public'.format(uname))
        else:
            click.echo('{} -> {}'.format(uname, locations[dest_id]['uname']))

    if pager:
        click.echo_via_pager(locations)
    else:
        click.echo(locations)

