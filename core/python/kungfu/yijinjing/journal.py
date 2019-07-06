import os
import shutil
import glob
import re
import pyyjj
import pandas as pd

JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}{}{}'.format(
    r'(.*)', os.sep,  # category
    r'(.*)', os.sep,  # group
    r'(.*)', os.sep,  # name
    r'journal', os.sep,  # mode
    r'(.*)', os.sep,  # mode
    r'(\w+).(\d+).journal',  # hash + page_id
)
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)

MODES = {
    'live': pyyjj.mode.LIVE,
    'data': pyyjj.mode.DATA,
    'replay': pyyjj.mode.REPLAY,
    'backtest': pyyjj.mode.BACKTEST,
    '*': pyyjj.mode.LIVE
}

CATEGORIES = {
    'md': pyyjj.category.MD,
    'td': pyyjj.category.TD,
    'strategy': pyyjj.category.STRATEGY,
    'system': pyyjj.category.SYSTEM,
    '*': pyyjj.category.SYSTEM
}


def find_mode(m):
    for k in MODES:
        if int(MODES[k]) == m:
            return MODES[k]
    return pyyjj.mode.LIVE


def find_category(c):
    for k in CATEGORIES:
        if int(CATEGORIES[k]) == c:
            return CATEGORIES[k]
    return pyyjj.category.SYSTEM


class Locator(pyyjj.locator):
    def __init__(self, home):
        pyyjj.locator.__init__(self)
        self._home = home

    def layout_dir(self, location, layout):
        mode = pyyjj.get_mode_name(location.mode)
        category = pyyjj.get_category_name(location.category)
        p = os.path.join(self._home, category, location.group, location.name, pyyjj.get_layout_name(layout), mode)
        if not os.path.exists(p):
            os.makedirs(p)
        return p

    def layout_file(self, location, layout, name):
        return os.path.join(self.layout_dir(location, layout), "{}.{}".format(name, pyyjj.get_layout_name(layout)))

    def default_to_system_db(self, location, name):
        file = os.path.join(self.layout_dir(location, pyyjj.layout.SQLITE), "{}.{}".format(name, pyyjj.get_layout_name(pyyjj.layout.SQLITE)))
        if os.path.exists(file):
            return file
        else:
            system_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "etc", "kungfu", self)
            system_file = os.path.join(self.layout_dir(system_location, pyyjj.layout.SQLITE),
                                       "{}.{}".format(name, pyyjj.get_layout_name(pyyjj.layout.SQLITE)))
            shutil.copy(system_file, file)
            return file

    def list_page_id(self, location, dest_id):
        page_ids = []
        for journal in glob.glob(os.path.join(self.layout_dir(location, pyyjj.layout.JOURNAL), hex(dest_id)[2:] + '.*.journal')):
            match = JOURNAL_LOCATION_PATTERN.match(journal[len(self._home) + 1:])
            if match:
                page_id = match.group(6)
                page_ids.append(int(page_id))
        return page_ids


def collect_journal_locations(ctx):
    search_path = os.path.join(ctx.home, ctx.category, ctx.group, ctx.name, 'journal', ctx.mode, '*.journal')

    locations = {}
    for journal in glob.glob(search_path):
        match = JOURNAL_LOCATION_PATTERN.match(journal[len(ctx.home) + 1:])
        if match:
            category = match.group(1)
            group = match.group(2)
            name = match.group(3)
            mode = match.group(4)
            dest = match.group(5)
            page_id = match.group(6)
            uname = '{}/{}/{}/{}'.format(category, group, name, mode)
            if uname in locations:
                if dest in locations[uname]['readers']:
                    locations[uname]['readers'][dest].append(page_id)
                else:
                    locations[uname]['readers'][dest] = [page_id]
            else:
                locations[uname] = {
                    'category': category,
                    'group': group,
                    'name': name,
                    'mode': mode,
                    'uname': uname,
                    'uid': pyyjj.hash_str_32(uname),
                    'readers': {
                        dest: [page_id]
                    }
                }
            ctx.logger.debug('found journal %s %s %s %s', MODES[mode], CATEGORIES[category], group, name)
        else:
            ctx.logger.warn('unable to match journal file %s to pattern %s', journal, JOURNAL_LOCATION_REGEX)

    return locations

def find_sessions(ctx):
    home = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, "master", "master", ctx.locator)
    io_device = pyyjj.io_device_client(home, False)

    ctx.session_count = 1
    sessions_df = pd.DataFrame(columns=[
        'id', 'mode', 'category', 'group', 'name', 'begin_time', 'end_time', 'closed', 'duration', 'frame_count'
    ])
    locations = collect_journal_locations(ctx)
    dest_pub = '{:08x}'.format(0)
    for key in locations:
        record = locations[key]
        location = pyyjj.location(MODES[record['mode']], CATEGORIES[record['category']], record['group'], record['name'], ctx.locator)
        if dest_pub in record['readers']:
            reader = io_device.open_reader(location, 0)
            find_sessions_from_reader(ctx, sessions_df, reader, record['mode'], record['category'], record['group'], record['name'])

    return sessions_df


def find_sessions_from_reader(ctx, sessions_df, reader, mode, category, group, name):
    session_start_time = -1
    last_frame_time = 0
    session_frame_count = 0

    while reader.data_available():
        frame = reader.current_frame()
        session_frame_count = session_frame_count + 1
        if frame.msg_type == 10001:
            if session_start_time > 0:
                sessions_df.loc[len(sessions_df)] = [
                    ctx.session_count, mode, category, group, name,
                    session_start_time, last_frame_time, False,
                    last_frame_time - session_start_time, session_frame_count
                ]
                session_start_time = frame.gen_time
                session_frame_count = 1
                ctx.session_count = ctx.session_count + 1
            else:
                session_start_time = frame.gen_time
                session_frame_count = 1
        elif frame.msg_type == 10002:
            if session_start_time > 0:
                sessions_df.loc[len(sessions_df)] = [
                    ctx.session_count, mode, category, group, name,
                    session_start_time, frame.gen_time, True,
                    frame.gen_time - session_start_time, session_frame_count
                ]
                session_start_time = -1
                session_frame_count = 0
                ctx.session_count = ctx.session_count + 1
        last_frame_time = frame.gen_time
        reader.next()

    if session_start_time > 0:
        sessions_df.loc[len(sessions_df)] = [
            ctx.session_count, mode, category, group, name,
            session_start_time, last_frame_time, False,
            last_frame_time - session_start_time, session_frame_count
        ]
