import os
import glob
import re
import kungfu.yijinjing.time as kft
import kungfu.command.journal as kfj
import pyyjj
from datetime import datetime, timedelta


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


@kfj.arg('-r', '--reverse', dest='reverse', action='store_true', help='reverse sorting')
@kfj.arg('-s', '--sortby', dest='sortby', type=str, default='begin_time',
         choices=['begin_time', 'end_time', 'duration', 'mode', 'category', 'group', 'name', 'frame_count'],
         help='sorting method')
@kfj.arg('-n', '--name', dest='name', type=str, default='*', help='name')
@kfj.arg('-g', '--group', dest='group', type=str, default='*', help='group')
@kfj.arg('-c', '--category', dest='category', type=str, default='*', choices=CATEGORIES.keys(), help='category')
@kfj.arg('-m', '--mode', dest='mode', type=str, default='*', choices=MODES.keys(), help='mode')
@kfj.command(help='list sessions, use * as wildcard')
def sessions(args, logger):
    all_sessions = find_sessions(args, logger)

    if not all_sessions:
        print('no sessions found')

    all_sessions = sorted(all_sessions, key=lambda s: s[args.sortby], reverse=args.reverse)
    for i in range(len(all_sessions)):
        session = all_sessions[i]
        session_meta = '[{:3d}] [{:^8}] [{:^8}] [{:^10}]'.format(i + 1, session['mode'], session['category'], session['group'])
        begin = kft.strftime(session['begin_time'], format=SESSION_DATETIME_FORMAT)
        end = kft.strftime(session['end_time'], format=SESSION_DATETIME_FORMAT)
        duration = kft.strftime(session['duration'] - DURATION_TZ_ADJUST, format=DURATION_FORMAT)
        session_info = '[ {} - {}{} [{}] [{:6d} frames]'.format(begin, end, ' ]' if session['closed'] else ' >', duration, session['frame_count'])
        print('{} {}'.format(session_meta, session_info))


def find_sessions(args, logger):
    pyyjj.setup_log(kfj.LOG_NAME)
    kf_home = os.environ['KF_HOME']
    search_path = os.path.join(kf_home, 'journal', args.mode, args.category, args.group, 'yjj.' + args.name + '.*.journal')

    io_device = pyyjj.create_io_device()

    all_sessions = []
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
            all_sessions.extend(find_sessions_from_reader(reader, mode, category, group, name))
        else:
            logger.warn('unable to match journal file %s to pattern %s', journal_location, JOURNAL_LOCATION_REGEX)
    return all_sessions


def find_sessions_from_reader(reader, mode, category, group, name):
    result = []
    session_start_time = -1
    last_frame_time = 0
    session_frame_count = 0

    while reader.data_available():
        frame = reader.current_frame()
        session_frame_count = session_frame_count + 1
        if frame.msg_type == 10001:
            if session_start_time > 0:
                result.append({
                    'mode': mode,
                    'category': category,
                    'group': group,
                    'name': name,
                    'begin_time': session_start_time,
                    'end_time': last_frame_time,
                    'duration': last_frame_time - session_start_time,
                    'closed': False,
                    'frame_count': session_frame_count
                })
                session_start_time = frame.gen_time
                session_frame_count = 1
            else:
                session_start_time = frame.gen_time
                session_frame_count = 1
        elif frame.msg_type == 10002:
            if session_start_time > 0:
                result.append({
                    'mode': mode,
                    'category': category,
                    'group': group,
                    'name': name,
                    'begin_time': session_start_time,
                    'end_time': frame.gen_time,
                    'duration': frame.gen_time - session_start_time,
                    'closed': True,
                    'frame_count': session_frame_count
                })
                session_start_time = -1
                session_frame_count = 0
        last_frame_time = frame.gen_time
        reader.next()

    if session_start_time > 0:
        result.append({
            'mode': mode,
            'category': category,
            'group': group,
            'name': name,
            'begin_time': session_start_time,
            'end_time': last_frame_time,
            'duration': last_frame_time - session_start_time,
            'closed': False,
            'frame_count': session_frame_count
        })
    return result
