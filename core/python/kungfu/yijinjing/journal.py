import os
import shutil
import glob
import re
import json
import pykungfu
from pykungfu import yijinjing as yjj
import pandas as pd
import errno
import kungfu.yijinjing.msg as yjj_msg

os_sep = re.escape(os.sep)
JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}'.format(
    r'(.*)', os_sep,  # category
    r'(.*)', os_sep,  # group
    r'(.*)', os_sep,  # name
    r'journal', os_sep,  # mode
    r'(.*)'
)
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)

JOURNAL_PAGE_REGEX = '{}{}{}{}{}{}{}{}{}{}{}'.format(
    r'(.*)', os_sep,  # category
    r'(.*)', os_sep,  # group
    r'(.*)', os_sep,  # name
    r'journal', os_sep,  # mode
    r'(.*)', os_sep,  # mode
    r'(\w+).(\d+).journal',  # hash + page_id
)
JOURNAL_PAGE_PATTERN = re.compile(JOURNAL_PAGE_REGEX)

LOCATION_UNAME_REGEX = r'(.*)/(.*)/(.*)/(.*)'
LOCATION_PATTERN = re.compile(LOCATION_UNAME_REGEX)

MODES = {
    'live': yjj.mode.LIVE,
    'data': yjj.mode.DATA,
    'replay': yjj.mode.REPLAY,
    'backtest': yjj.mode.BACKTEST,
    '*': yjj.mode.LIVE
}

CATEGORIES = {
    'md': yjj.category.MD,
    'td': yjj.category.TD,
    'strategy': yjj.category.STRATEGY,
    'system': yjj.category.SYSTEM,
    '*': yjj.category.SYSTEM
}


def find_mode(m):
    for k in MODES:
        if int(MODES[k]) == m:
            return MODES[k]
    return yjj.mode.LIVE


def find_category(c):
    for k in CATEGORIES:
        if int(CATEGORIES[k]) == c:
            return CATEGORIES[k]
    return yjj.category.SYSTEM


def get_location_from_json(ctx, data):
    if 'mode' in data and 'category' in data and 'group' in data and 'name' in data:
        return yjj.location(MODES[data['mode']], CATEGORIES[data['category']], data['group'], data['name'], ctx.locator)
    else:
        return None


class Locator(yjj.locator):
    def __init__(self, home):
        yjj.locator.__init__(self)
        self._home = home

    def has_env(self, name):
        return os.getenv(name) is not None

    def get_env(self, name):
        return os.getenv(name)

    def layout_dir(self, location, layout):
        mode = yjj.get_mode_name(location.mode)
        category = yjj.get_category_name(location.category)
        p = os.path.join(self._home, category, location.group, location.name, yjj.get_layout_name(layout), mode)
        try:
            os.makedirs(p)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
        return p

    def layout_file(self, location, layout, name):
        return os.path.join(self.layout_dir(location, layout), "{}.{}".format(name, yjj.get_layout_name(layout)))

    def default_to_system_db(self, location, name):
        file = os.path.join(self.layout_dir(location, yjj.layout.SQLITE), "{}.{}".format(name, yjj.get_layout_name(yjj.layout.SQLITE)))
        if os.path.exists(file):
            return file
        else:
            system_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, "etc", "kungfu", self)
            system_file = os.path.join(self.layout_dir(system_location, yjj.layout.SQLITE),
                                       "{}.{}".format(name, yjj.get_layout_name(yjj.layout.SQLITE)))
            shutil.copy(system_file, file)
            return file

    def list_page_id(self, location, dest_id):
        page_ids = []
        for journal in glob.glob(os.path.join(self.layout_dir(location, yjj.layout.JOURNAL), hex(dest_id)[2:] + '.*.journal')):
            match = JOURNAL_PAGE_PATTERN.match(journal[len(self._home) + 1:])
            if match:
                page_id = match.group(6)
                page_ids.append(int(page_id))
        return page_ids

    def list_locations(self, category, group, name, mode):
        search_path = os.path.join(self._home, category, group, name, 'journal', mode)
        locations = []
        for journal in glob.glob(search_path):
            match = JOURNAL_LOCATION_PATTERN.match(journal[len(self._home) + 1:])
            if match:
                category = match.group(1)
                group = match.group(2)
                name = match.group(3)
                mode = match.group(4)
                location = yjj.location(MODES[mode], CATEGORIES[category], group, name, self)
                locations.append(location)
        return locations

    def list_location_dest(self, location):
        search_path = os.path.join(self._home, yjj.get_category_name(location.category),
                                   location.group, location.name, 'journal',
                                   yjj.get_mode_name(location.mode), '*.journal')
        readers = {}
        for journal in glob.glob(search_path):
            match = JOURNAL_PAGE_PATTERN.match(journal[len(self._home) + 1:])
            if match:
                category = match.group(1)
                group = match.group(2)
                name = match.group(3)
                mode = match.group(4)
                dest = match.group(5)
                page_id = match.group(6)
                uname = '{}/{}/{}/{}'.format(category, group, name, mode)
                uid = yjj.hash_str_32(uname)
                if dest in readers:
                    readers[int(dest, 16)].append(page_id)
                else:
                    readers[int(dest, 16)] = [page_id]
        return list(readers.keys())


def collect_journal_locations(ctx):
    search_path = os.path.join(ctx.home, ctx.category, ctx.group, ctx.name, 'journal', ctx.mode, '*.journal')
    locations = {}
    for journal in glob.glob(search_path):
        match = JOURNAL_PAGE_PATTERN.match(journal[len(ctx.home) + 1:])
        if match:
            category = match.group(1)
            group = match.group(2)
            name = match.group(3)
            mode = match.group(4)
            dest = match.group(5)
            page_id = match.group(6)
            uname = '{}/{}/{}/{}'.format(category, group, name, mode)
            uid = yjj.hash_str_32(uname)
            if uid in locations:
                if dest in locations[uid]['readers']:
                    locations[uid]['readers'][dest].append(page_id)
                else:
                    locations[uid]['readers'][dest] = [page_id]
            else:
                locations[uid] = {
                    'category': category,
                    'group': group,
                    'name': name,
                    'mode': mode,
                    'uname': uname,
                    'uid': yjj.hash_str_32(uname),
                    'readers': {
                        dest: [page_id]
                    }
                }
            ctx.logger.debug('found journal %s %s %s %s', MODES[mode], CATEGORIES[category], group, name)
        else:
            ctx.logger.warn('unable to match journal file %s to pattern %s', journal, JOURNAL_PAGE_REGEX)
    return locations


def find_sessions(ctx):
    io_device = yjj.io_device(ctx.journal_util_location)
    session_keeper = yjj.session_keeper(io_device)
    ctx.session_count = 1
    sessions_df = pd.DataFrame(columns=[
        'id', 'mode', 'category', 'group', 'name', 'begin_time', 'end_time', 'closed', 'duration'
    ])
    for session in session_keeper.find_sessions():
        sessions_df.loc[len(sessions_df)] = [
            len(sessions_df) + 1, yjj.get_mode_name(session.mode), yjj.get_category_name(session.category), session.group, session.name,
            session.begin_time, session.end_time, session.end_time > 0,
            session.end_time - session.begin_time
        ]
    return sessions_df


def find_session(ctx, session_id):
    all_sessions = find_sessions(ctx)
    return all_sessions[all_sessions['id'] == session_id].iloc[0]


def make_location_from_dict(ctx, location):
    return yjj.location(MODES[location['mode']], CATEGORIES[location['category']], location['group'], location['name'], ctx.locator)


def read_session(ctx, session_id, io_type):
    session = find_session(ctx, session_id)
    uname = '{}/{}/{}/{}'.format(session['category'], session['group'], session['name'], session['mode'])
    print(uname)
    uid = yjj.hash_str_32(uname)
    ctx.category = '*'
    ctx.group = '*'
    ctx.name = '*'
    ctx.mode = '*'
    locations = collect_journal_locations(ctx)
    location = locations[uid]
    home = make_location_from_dict(ctx, location)
    io_device = yjj.io_device_console(home, ctx.console_width, ctx.console_height)

    show_in = (io_type == 'in' or io_type == 'all') and not (io_device.home.category == yjj.category.SYSTEM and io_device.home.group == 'master' and io_device.home.name == 'master')
    show_out = io_type == 'out' or io_type == 'all'

    return locations, session, io_device, show_in, show_out


def show_journal(ctx, session_id, io_type):
    locations, session, io_device, show_in, show_out = read_session(ctx, session_id, io_type)

    reader = io_device.open_reader_to_subscribe()

    if show_in:
        for dest_id in ctx.locator.list_location_dest(io_device.home):
            reader.join(io_device.home, dest_id, session['begin_time'])

    if show_out:
        master_home_uid = yjj.hash_str_32('system/master/master/live')
        master_home_location = make_location_from_dict(ctx, locations[master_home_uid])
        reader.join(master_home_location, 0, session['begin_time'])

        master_cmd_uid = yjj.hash_str_32('system/master/{:08x}/live'.format(io_device.home.uid))
        master_cmd_location = make_location_from_dict(ctx, locations[master_cmd_uid])
        reader.join(master_cmd_location, io_device.home.uid, session['begin_time'])

    journal_df = pd.DataFrame(columns=[
        'gen_time', 'trigger_time', 'source', 'dest', 'msg_type', 'frame_length', 'data_length'
    ])
    while reader.data_available() and reader.current_frame().gen_time <= session['end_time']:
        frame = reader.current_frame()
        journal_df.loc[len(journal_df)] = [
            frame.gen_time, frame.trigger_time,
            locations[frame.source]['uname'],
            'public' if frame.dest == 0 else locations[frame.dest]['uname'],
            frame.msg_type, frame.frame_length, frame.data_length
        ]
        if frame.dest == io_device.home.uid and (frame.msg_type == yjj_msg.RequestReadFrom or frame.msg_type == yjj_msg.RequestReadFromPublic):
            request = frame.RequestReadFrom()
            source_location = make_location_from_dict(ctx, locations[request.source_id])
            dest = io_device.home.uid if frame.msg_type == yjj_msg.RequestReadFrom else 0
            try:
                reader.join(source_location, dest, request.from_time)
            except Exception as err:
                ctx.logger.error(f"failed to join journal {source_location.uname}/{dest}, exception: {err}")
        if frame.dest == io_device.home.uid and frame.msg_type == yjj_msg.Deregister:
            loc = json.loads(frame.data_as_string())
            reader.disjoin(loc['uid'])
        reader.next()
    return journal_df


def trace_journal(ctx, session_id, io_type):
    locations, session, io_device, show_in, show_out = read_session(ctx, session_id, io_type)
    io_device.trace(session['begin_time'], session['end_time'], show_in, show_out)
