import pyyjj
import json
import click
from test import test, pass_ctx_from_parent
from kungfu.yijinjing import msg
from kungfu.yijinjing.log import create_logger
from . import calendar
from . import order

test_account = '15040900'
test_strategy = 'test1'


@test.command()
@click.pass_context
def ledger(ctx):
    pass_ctx_from_parent(ctx)
    commander_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'service', 'ledger', ctx.locator)
    ctx.logger = create_logger("ledger_test", ctx.log_level, commander_location)
    io_device = pyyjj.io_device(commander_location)
    cmd_sock = io_device.connect_socket(commander_location, pyyjj.protocol.REQUEST, 10000)

    calendar.calendar_request(cmd_sock, commander_location)

    # order.new_order_single(cmd_sock, commander_location, test_account)
    # ctx.logger.info('done')
    order.cancel_all_order_for_account(cmd_sock, commander_location, test_account)
    ctx.logger.info('done')

    # order.cancel_all_order_for_strategy(cmd_sock, commander_location, test_strategy)
    # ctx.logger.info('done')
