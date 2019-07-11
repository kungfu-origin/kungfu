import pyyjj
import json
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.log import create_logger


@kfc.group()
@click.pass_context
def test(ctx):
    pass_ctx_from_root(ctx)
    ctx.test_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'test', 'test', ctx.locator)
    ctx.io_device = pyyjj.io_device(ctx.test_location)


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.test_location = ctx.parent.test_location
    ctx.io_device = ctx.parent.io_device


def send_to_passive(ctx, sock, location, data):
    sock.send(json.dumps({
        'gen_time': pyyjj.now_in_nano(),
        'trigger_time': 0,
        'msg_type': 10099,
        'source': ctx.io_device.home.uid,
        'dest': location.uid,
        'data': data
    }))
