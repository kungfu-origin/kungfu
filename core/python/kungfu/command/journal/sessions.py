import pandas as pd
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
@click.option('-s', '--sortby', default='begin_time',
              type=click.Choice(['begin_time', 'end_time', 'duration', 'mode', 'category', 'group', 'name', 'frame_count']),
              help='sorting method')
@click.option('-a', '--ascending', is_flag=True, help='sorted as ascending')
@click.option('-f', '--tablefmt', default='simple',
              type=click.Choice(['plain', 'simple', 'orgtbl', 'grid', 'fancy_grid', 'rst', 'textile']),
              help='output format')
@click.option('-p', '--pager', is_flag=True, help='show in a pager')
@click.pass_context
def sessions(ctx, sortby, ascending, tablefmt, pager):
    pass_ctx_from_parent(ctx)
    all_sessions = find_sessions(ctx).sort_values(by=sortby, ascending=ascending)
    all_sessions['begin_time'] = all_sessions['begin_time'].apply(lambda t: kft.strftime(t, SESSION_DATETIME_FORMAT))
    all_sessions['end_time'] = all_sessions['end_time'].apply(lambda t: kft.strftime(t, SESSION_DATETIME_FORMAT))
    all_sessions['duration'] = all_sessions['duration'].apply(lambda t: kft.strftime(t - DURATION_TZ_ADJUST, DURATION_FORMAT))

    table = tabulate(all_sessions.values, headers=all_sessions.columns, tablefmt=tablefmt)
    if pager:
        click.echo_via_pager(table)
    else:
        click.echo(table)


def find_sessions(ctx):
    home = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "master", "master", ctx.locator)
    io_device = pyyjj.io_device_client(home, False)

    sessions_df = pd.DataFrame(columns=[
        'mode', 'category', 'group', 'name', 'begin_time', 'end_time', 'closed', 'duration', 'frame_count'
    ])
    locations = kfj.collect_journal_locations(ctx)
    dest_pub = '{:08x}'.format(0)
    for key in locations:
        record = locations[key]
        location = pyyjj.location(kfj.MODES[record['mode']], kfj.CATEGORIES[record['category']], record['group'], record['name'], ctx.locator)
        if dest_pub in record['readers']:
            reader = io_device.open_reader(location, 0)
            find_sessions_from_reader(sessions_df, reader, record['mode'], record['category'], record['group'], record['name'])

    return sessions_df


def find_sessions_from_reader(sessions_df, reader, mode, category, group, name):
    session_start_time = -1
    last_frame_time = 0
    session_frame_count = 0

    while reader.data_available():
        frame = reader.current_frame()
        session_frame_count = session_frame_count + 1
        if frame.msg_type == 10001:
            if session_start_time > 0:
                sessions_df.loc[len(sessions_df)] = [
                    mode, category, group, name,
                    session_start_time, last_frame_time, False,
                    last_frame_time - session_start_time, session_frame_count
                ]
                session_start_time = frame.gen_time
                session_frame_count = 1
            else:
                session_start_time = frame.gen_time
                session_frame_count = 1
        elif frame.msg_type == 10002:
            if session_start_time > 0:
                sessions_df.loc[len(sessions_df)] = [
                    mode, category, group, name,
                    session_start_time, frame.gen_time, True,
                    frame.gen_time - session_start_time, session_frame_count
                ]
                session_start_time = -1
                session_frame_count = 0
        last_frame_time = frame.gen_time
        reader.next()

    if session_start_time > 0:
        sessions_df.loc[len(sessions_df)] = [
            mode, category, group, name,
            session_start_time, last_frame_time, False,
            last_frame_time - session_start_time, session_frame_count
        ]
