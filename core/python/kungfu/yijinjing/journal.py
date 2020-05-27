import json
import pandas
from kungfu.yijinjing import *
from pykungfu.longfist.types import *
from pykungfu import yijinjing as yjj


def collect_journal_locations(ctx):
    search_path = os.path.join(ctx.runtime_dir, ctx.category, ctx.group, ctx.name, 'journal', ctx.mode, '*.journal')
    locations = {}
    for journal in glob.glob(search_path):
        match = JOURNAL_PAGE_PATTERN.match(journal[len(ctx.runtime_dir) + 1:])
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
    session_finder = yjj.session_finder(io_device)
    ctx.session_count = 1
    sessions_df = pandas.DataFrame(columns=[
        'id', 'mode', 'category', 'group', 'name', 'begin_time', 'end_time', 'closed', 'duration'
    ])
    for_app = 'app_location' in dir(ctx)
    sessions = session_finder.find_sessions_for(ctx.app_location) if for_app else session_finder.find_sessions()
    for session in sessions:
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
    return yjj.location(MODES[location['mode']], CATEGORIES[location['category']], location['group'], location['name'], ctx.runtime_locator)


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

    show_in = (io_type == 'in' or io_type == 'all') and not (
            io_device.home.category == yjj.category.SYSTEM and io_device.home.group == 'master' and io_device.home.name == 'master')
    show_out = io_type == 'out' or io_type == 'all'

    return locations, session, io_device, show_in, show_out


def show_journal(ctx, session_id, io_type):
    locations, session, io_device, show_in, show_out = read_session(ctx, session_id, io_type)

    reader = io_device.open_reader_to_subscribe()

    if show_in:
        for dest_id in ctx.runtime_locator.list_location_dest(io_device.home):
            reader.join(io_device.home, dest_id, session['begin_time'])

    if show_out:
        master_home_uid = yjj.hash_str_32('system/master/master/live')
        master_home_location = make_location_from_dict(ctx, locations[master_home_uid])
        reader.join(master_home_location, 0, session['begin_time'])

        master_cmd_uid = yjj.hash_str_32('system/master/{:08x}/live'.format(io_device.home.uid))
        master_cmd_location = make_location_from_dict(ctx, locations[master_cmd_uid])
        reader.join(master_cmd_location, io_device.home.uid, session['begin_time'])

    journal_df = pandas.DataFrame(columns=[
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
        if frame.dest == io_device.home.uid and (frame.msg_type == RequestReadFrom.tag or frame.msg_type == RequestReadFromPublic.tag):
            request = frame.RequestReadFrom()
            source_location = make_location_from_dict(ctx, locations[request.source_id])
            dest = io_device.home.uid if frame.msg_type == RequestReadFrom.tag else 0
            try:
                reader.join(source_location, dest, request.from_time)
            except Exception as err:
                ctx.logger.error(f"failed to join journal {source_location.uname}/{dest}, exception: {err}")
        if frame.dest == io_device.home.uid and frame.msg_type == Deregister.tag:
            loc = json.loads(frame.data_as_string())
            reader.disjoin(loc['uid'])
        reader.next()
    return journal_df


def trace_journal(ctx, session_id, io_type):
    locations, session, io_device, show_in, show_out = read_session(ctx, session_id, io_type)
    io_device.trace(session['begin_time'], session['end_time'], show_in, show_out)

def prune_journals(ctx, base_dir):
    search_path = os.path.join(base_dir, ctx.category, ctx.group, ctx.name, 'journal', ctx.mode, '*.journal')
    for journal in glob.glob(search_path):
        match = JOURNAL_PAGE_PATTERN.match(journal[len(base_dir) + 1:])
        if match:
            category = match.group(1)
            group = match.group(2)
            name = match.group(3)
            mode = match.group(4)
            dest = match.group(5)
            page_id = match.group(6)
            print(f'pruning {category}/{group}/{name}/{mode}')
        else:
            ctx.logger.warn('unable to match journal file %s to pattern %s', journal, JOURNAL_PAGE_REGEX)
