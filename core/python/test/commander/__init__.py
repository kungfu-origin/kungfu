import pyyjj
import json
import click
from test import test, pass_ctx_from_parent
from kungfu.yijinjing import msg


@test.command()
@click.pass_context
def commander(ctx):
    pass_ctx_from_parent(ctx)
    commander_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'commander', 'commander', ctx.locator)
    io_device = pyyjj.io_device(commander_location)
    cmd_sock = io_device.connect_socket(commander_location, pyyjj.protocol.REQUEST, 1000)
    cmd_sock.send(json.dumps({
        'gen_time': pyyjj.now_in_nano(),
        'trigger_time': 0,
        'msg_type': msg.UIActionCancelAllOrder,
        'source': 0,
        'dest': commander_location.uid,
        'data': {}
    }))
    cmd_sock.recv()
    print('commander test', cmd_sock.last_message())
