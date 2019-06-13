import os
import glob
import re
import pandas as pd
import kungfu.yijinjing.time as kft
import kungfu.command.journal as kfj
import pyyjj
from datetime import datetime, timedelta
from tabulate import tabulate
import click


SESSION_DATETIME_FORMAT = '%Y-%m-%d %H:%M:%S'
DURATION_FORMAT = '%H:%M:%S.%N'
DURATION_TZ_ADJUST = int(timedelta(hours=datetime.fromtimestamp(0).hour).total_seconds() * 1e9)


JOURNAL_FILE_SIZE = int(128 * 1024 * 1024)
JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}'.format(r'(.*)', os.sep,  # journal
                                                     r'(.*)', os.sep,  # mode
                                                     r'(.*)', os.sep,  # category
                                                     r'(.*)', os.sep,  # group
                                                     r'yjj.(.*).(\d+).journal',  # name + page_id
                                                     )
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)


MODES = {
    'live': pyyjj.mode.LIVE,
    'data': pyyjj.mode.DATA,
    'replay': pyyjj.mode.REPLAY,
    'backtest': pyyjj.mode.BACKTEST,
    '*': 0
}

CATEGORIES = {
    'md': pyyjj.category.MD,
    'td': pyyjj.category.TD,
    'strategy': pyyjj.category.STRATEGY,
    'system': pyyjj.category.SYSTEM,
    '*': 0
}


@kfj.arg('-p', '--pager', dest='pager', action='store_true', help='show in a pager')
@kfj.arg('-f', '--format', dest='format', default='simple', choices=['plain', 'simple', 'orgtbl', 'grid', 'fancy_grid', 'rst', 'textile'], help='output format')
@kfj.arg('-a', '--ascending', dest='ascending', action='store_true', help='sorted as ascending')
@kfj.arg('-s', '--sortby', dest='sortby', type=str, default='begin_time',
         choices=['begin_time', 'end_time', 'duration', 'mode', 'category', 'group', 'name', 'frame_count'],
         help='sorting method')
@kfj.arg('-n', '--name', dest='name', type=str, default='*', help='name')
@kfj.arg('-g', '--group', dest='group', type=str, default='*', help='group')
@kfj.arg('-c', '--category', dest='category', type=str, default='*', choices=CATEGORIES.keys(), help='category')
@kfj.arg('-m', '--mode', dest='mode', type=str, default='*', choices=MODES.keys(), help='mode')
@kfj.command(help='list sessions, use * as wildcard')
def sessions(args, logger):

    all_sessions = find_sessions(args, logger).sort_values(by=args.sortby, ascending=args.ascending)
    all_sessions['begin_time'] = all_sessions['begin_time'].apply(lambda t: kft.strftime(t, SESSION_DATETIME_FORMAT))
    all_sessions['end_time'] = all_sessions['end_time'].apply(lambda t: kft.strftime(t, SESSION_DATETIME_FORMAT))
    all_sessions['duration'] = all_sessions['duration'].apply(lambda t: kft.strftime(t - DURATION_TZ_ADJUST, DURATION_FORMAT))

    table = tabulate(all_sessions.values, headers=all_sessions.columns, tablefmt=args.format)
    if args.pager:
        click.echo_via_pager(table)
    else:
        print(table)


def find_sessions(args, logger):
    pyyjj.setup_log(kfj.LOG_NAME)
    kf_home = os.environ['KF_HOME']
    search_path = os.path.join(kf_home, 'journal', args.mode, args.category, args.group, 'yjj.' + args.name + '.*.journal')

    io_device = pyyjj.create_io_device()

    sessions_df = pd.DataFrame(columns=[
        'mode', 'category', 'group', 'name', 'begin_time', 'end_time', 'closed', 'duration', 'frame_count'
    ])
    for journal in glob.glob(search_path):
        if os.path.getsize(journal) != JOURNAL_FILE_SIZE:
            logger.error('bad journal file %s', journal)
            pass
        journal_location = journal[len(kf_home) + 1:]
        match = JOURNAL_LOCATION_PATTERN.match(journal_location)
        if match:
            mode = match.group(2)
            category = match.group(3)
            group = match.group(4)
            name = match.group(5)
            logger.debug('subscribe to %s %s %s %s', MODES[mode], CATEGORIES[category], group, name)
            reader = io_device.open_reader(MODES[mode], CATEGORIES[category], group, name)
            find_sessions_from_reader(sessions_df, reader, mode, category, group, name)
        else:
            logger.warn('unable to match journal file %s to pattern %s', journal_location, JOURNAL_LOCATION_REGEX)
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
