import pyyjj
import json
import click
from test import test, pass_ctx_from_parent
from kungfu.yijinjing import msg
from . import order

test_account = '15040900'
test_strategy = 'test1'


@test.command()
@click.pass_context
def commander(ctx):
    pass_ctx_from_parent(ctx)
    commander_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'commander', 'commander', ctx.locator)
    io_device = pyyjj.io_device(commander_location)
    cmd_sock = io_device.connect_socket(commander_location, pyyjj.protocol.REQUEST, 1000)

    order.new_order_single(cmd_sock, commander_location, test_account)
    order.cancel_all_order_for_account(cmd_sock, commander_location, test_account)

    order.cancel_all_order_for_strategy(cmd_sock, commander_location, test_strategy)
