import pyyjj
import json
import click
from test import test, pass_ctx_from_parent
from kungfu.yijinjing import msg
from kungfu.yijinjing.log import create_logger
from . import calendar
from . import order
from . import asset
from . import broker

test_account = '15040900'
test_strategy = 'test1'


@test.command()
@click.option('-p', '--protocol', type=str, help='protocol', default="req", show_default=True)
@click.option('-m', '--msg_type', type=int, help="msg_type", default=0, show_default=True)
@click.pass_context
def ledger(ctx, protocol, msg_type):
    pass_ctx_from_parent(ctx)
    commander_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'service', 'ledger', ctx.locator)
    ctx.logger = create_logger("ledger_test", ctx.log_level, commander_location)
    io_device = pyyjj.io_device(commander_location)

    cmd_sock = io_device.connect_socket(commander_location, pyyjj.protocol.SUBSCRIBE if protocol == 'sub' else pyyjj.protocol.REQUEST, 10000)
    if protocol == "sub":
        cmd_sock.setsockopt(int(pyyjj.protocol.SUBSCRIBE), 1, "")
        while True:
            cmd_sock.recv()
            print(cmd_sock.last_message())
    calendar.calendar_request(cmd_sock, commander_location)
    broker.broker_state_request(cmd_sock, commander_location)
    order.cancel_all_order_for_account(cmd_sock, commander_location, test_account)
    asset.asset_request(cmd_sock, commander_location)
    asset.all_asset_info_request(cmd_sock, commander_location)
    ctx.logger.info('done')

    # order.cancel_all_order_for_strategy(cmd_sock, commander_location, test_strategy)
    # ctx.logger.info('done')
