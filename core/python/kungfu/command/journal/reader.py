
import pyyjj
import pywingchun
import click
from kungfu.command.journal import journal, pass_ctx_from_parent
import kungfu.yijinjing.msg as yjj_msg
import kungfu.yijinjing.journal as kfj
import kungfu.yijinjing.time as kft
import time
import sys
import csv
import traceback
import pprint
import importlib
import os
import kungfu.msg

@journal.command()
@click.option('-i', '--session_id', type=int, required=True, help='session id')
@click.option('-t', '--io_type', type=click.Choice(['all', 'in', 'out']), default='all', help='input or output during this session')
@click.option("--from-beginning", is_flag=True, help="start with the earliest message within this session")
@click.option("--max-messages", type=int, default=sys.maxsize, help="The maximum number of messages to reader before exiting")
@click.option('--msg', type=click.Choice(['all'] + kungfu.msg.Registry.type_names()), default='all',help="msg type to read")
@click.option("--continuous", is_flag=True, help="reader not to stop when no data avail util the session end")
@click.option("-o", "--output", type=str,help="file path where you want to store the exported csv file")
@click.option('--script', type=str, required=False, help="python script to process every frame data")
@click.pass_context
def reader(ctx, session_id, io_type, from_beginning, max_messages, msg, continuous, output, script):
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
    home = kfj.make_location_from_dict(ctx, location)
    io_device = pyyjj.io_device(home)
    reader = io_device.open_reader_to_subscribe()
    if io_type == 'out' or io_type == 'all':
        for dest in location['readers']:
            dest_id = int(dest, 16)
            reader.join(home, dest_id, session['begin_time'])

    if (io_type == 'in' or io_type == 'all') and not (home.category == pyyjj.category.SYSTEM and home.group == 'master' and home.name == 'master'):
        master_home_uid = pyyjj.hash_str_32('system/master/master/live')
        master_home_location = kfj.make_location_from_dict(ctx, locations[master_home_uid])
        reader.join(master_home_location, 0, session['begin_time'])

        master_cmd_uid = pyyjj.hash_str_32('system/master/{:08x}/live'.format(location['uid']))
        master_cmd_location = kfj.make_location_from_dict(ctx, locations[master_cmd_uid])
        reader.join(master_cmd_location, location['uid'], session['begin_time'])

    start_time = pyyjj.now_in_nano() if not from_beginning else session["begin_time"]
    msg_count = 0
    msg_type_to_read = None if msg == "all" else kungfu.msg.Registry.meta_from_name(msg)["id"]
    if output:
        if msg not in kungfu.msg.Registry.type_names():
            raise ValueError("invalid msg {}, please choose from {}".format(kungfu.msg.Registry.type_names()))
        csv_writer = None
        def handle(frame):
            data_as_dict = frame["data"]
            dict_row = kungfu.msg.utils.flatten_json(data_as_dict)
            nonlocal csv_writer
            if not csv_writer:
                csv_writer = csv.DictWriter(open(output, "w"), fieldnames = dict_row.keys())
                csv_writer.writeheader()
            csv_writer.writerow(dict_row)
        frame_handler = handle
    elif script:
        dir = os.path.dirname(script)
        name_no_ext = os.path.split(os.path.basename(script))
        sys.path.append(os.path.relpath(dir))
        impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
        frame_handler = getattr(impl, 'on_frame', lambda frame: None)
    else:
        pp = pprint.PrettyPrinter(indent=4)
        frame_handler = pp.pprint

    while True:
        if reader.data_available() and msg_count < max_messages:
            frame = reader.current_frame()
            if frame.dest == home.uid and (frame.msg_type == yjj_msg.RequestReadFrom or frame.msg_type == yjj_msg.RequestReadFromPublic):
                request = pyyjj.get_RequestReadFrom(frame)
                source_location = kfj.make_location_from_dict(ctx, locations[request.source_id])
                reader.join(source_location, location['uid'] if frame.msg_type == yjj_msg.RequestReadFrom else 0, request.from_time)
            if frame.dest == home.uid and frame.msg_type == yjj_msg.Deregister:
                loc = json.loads(frame.data_as_string())
                reader.disjoin(loc['uid'])
            if frame.msg_type == yjj_msg.SessionEnd:
                ctx.logger.info("session reach end at %s", kft.strftime(frame.gen_time))
                break
            elif frame.gen_time >= start_time and (msg == "all" or msg_type_to_read ==  frame.msg_type):
                try:
                    frame_handler(frame.as_dict())
                except Exception as e:
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
                msg_count +=1
            reader.next()
        elif msg_count >= max_messages:
            ctx.logger.info("reach max messages {}".format(max_messages))
            break
        elif not reader.data_available():
            if not continuous:
                ctx.logger.info("no data is available")
                break
            else:
                time.sleep(0.1)

