import os
import sys
import traceback
import glob
import re
import kungfu.yijinjing.io as kfio
import kungfu.yijinjing.time as kft
import kungfu.command.journal as kfj
import pyyjj


TOOLS_LOG_NAME = 'journal_tools'
SESSION_DATETIME_FORMAT = '%Y-%m-%d %H:%M:%S'

JOURNAL_FILE_SIZE = int(128 * 1024 * 1024)
JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}'.format(r'(.*)', os.sep,  # journal
                                                     r'(.*)', os.sep,  # mode
                                                     r'(.*)', os.sep,  # category
                                                     r'(.*)', os.sep,  # group
                                                     r'yjj.(.*).(\d+).journal',  # name + page_id
                                                     )
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)

JOURNALS = {}

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


@kfj.arg('-n', '--name', dest='jname', type=str, default='*', help='name')
@kfj.arg('-g', '--group', dest='group', type=str, default='*', help='group')
@kfj.arg('-c', '--category', dest='category', type=str, default='*', choices=CATEGORIES.keys(), help='category')
@kfj.arg('-m', '--mode', dest='mode', type=str, default='*', choices=MODES.keys(), help='mode')
@kfj.command(help='list sessions, use * as wildcard')
def sessions(args, logger):
    find_sessions(args, logger)


def find_sessions(args, logger):
    kf_home = os.environ['KF_HOME']
    search_path = os.path.join(kf_home, 'journal', args.mode, args.category, args.group, 'yjj.' + args.jname + '.*.journal')

    io_device = pyyjj.create_io_device_client(pyyjj.setup_log(TOOLS_LOG_NAME), False)
    kfio.checkin(args, logger, io_device)

    try:
        reader = io_device.open_reader_to_subscribe()

        for journal in glob.glob(search_path):
            if os.path.getsize(journal) != JOURNAL_FILE_SIZE:
                pass
            journal_location = journal[len(kf_home) + 1:]
            match = JOURNAL_LOCATION_PATTERN.match(journal_location)
            if match:
                mode = match.group(2)
                category = match.group(3)
                group = match.group(4)
                name = match.group(5)
                logger.info('subscribe to %s %s %s %s', MODES[mode], CATEGORIES[category], group, name)
                reader.subscribe(MODES[mode], CATEGORIES[category], group, name, 0)

        sessions = reader.find_sessions_from_current_frame()
        if not sessions:
            print('no sessions found')
        for i in range(len(sessions)):
            s = sessions[i]
            mode_name = '{}'.format(s.location.mode)[5:].lower()
            category_name = '{}'.format(s.location.category)[9:].lower()
            session_meta = '[{:2d}] [{:^8}] [{:^8}] [{:^10}]'.format(i + 1, mode_name, category_name, s.location.group)
            begin = kft.strftime(s.begin_time, format=SESSION_DATETIME_FORMAT)
            end = kft.strftime(s.end_time, format=SESSION_DATETIME_FORMAT)
            session_info = '[ {} - {}{} [{:4d} frames]'.format(begin, end, ' ]' if s.closed else ' >', s.frame_count)
            print('{} {}'.format(session_meta, session_info))
    except:
        exc_type, exc_obj, exc_tb = sys.exc_info()
        logger.error('failed to find session for %s, %s %s', search_path, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    kfio.checkout(args, logger, io_device)

