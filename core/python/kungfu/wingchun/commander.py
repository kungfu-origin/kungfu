import pywingchun
import pyyjj
import json
import functools
import kungfu.yijinjing.journal as kfj
from kungfu.yijinjing import msg
from kungfu.yijinjing.log import create_logger

HANDLERS = dict()


def on(msg_type):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        HANDLERS[msg_type] = handler_wrapper
        return on
    return register_handler


def handle(msg_type, *args, **kwargs):
    if msg_type not in HANDLERS:
        args[0].logger.error("invalid msg_type %s", msg_type)
    return HANDLERS[msg_type](*args, **kwargs)


class Commander(pywingchun.Commander):
    def __init__(self, ctx):
        pywingchun.Commander.__init__(self, ctx.locator, ctx.low_latency, ctx.name)
        self.ctx = ctx
        self.ctx.logger = create_logger(ctx.name, ctx.log_level, self.io_device.home)
        self.orders = {}

    def handle_request(self, msg):
        req = json.loads(msg)
        self.ctx.logger.debug("handle request %s", msg)
        self.ctx.logger.debug("handle request json %s", json.dumps(req))
        handle(req['msg_type'], self.ctx, req)
        return msg

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s from %s', order, self.get_location(event.dest).uname)
        self.orders[order.order_id] = order


@on(msg.UIActionNewOrderSingle)
def cancel_order(ctx, event):
    data = event['data']


@on(msg.UIActionCancelOrder)
def cancel_order(ctx, event):
    data = event['data']


@on(msg.UIActionCancelAllOrder)
def cancel_all_order(ctx, event):
    data = event['data']
    location = kfj.get_location_from_json(ctx, data)
    ctx.logger.warn('canceling all order for %s' % location.uid)
