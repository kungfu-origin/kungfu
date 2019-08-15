import pywingchun
import pyyjj
import json
import http
import functools
import kungfu.yijinjing.journal as kfj
from kungfu.wingchun import msg
from kungfu.yijinjing.log import create_logger
from kungfu.wingchun.calendar import Calendar

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
        ctx.calendar = Calendar(ctx)
        self.ctx = ctx
        self.ctx.commander = self
        self.ctx.logger = create_logger(ctx.name, ctx.log_level, self.io_device.home)
        self.ctx.orders = {}

    def handle_request(self, event, msg):
        req = json.loads(msg)
        data = req['data']
        location = kfj.get_location_from_json(self.ctx, data)
        return json.dumps(handle(req['msg_type'], self.ctx, event, location))

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s from %s', order, self.get_location(event.dest).uname)
        order_record = {
            'source': event.source,
            'dest': event.dest,
            'order': order
        }
        self.ctx.orders[order.order_id] = order_record


@on(msg.CalendarRequest)
def calendar_request(ctx, event, location):
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.Calendar,
        'data': {
            'trading_day': '%s' % ctx.calendar.trading_day
        }
    }


@on(msg.NewOrderSingle)
def new_order_single(ctx, event, location):
    # ctx.commander.new_order_single(event, location.uid)
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.NewOrderSingle
    }


@on(msg.CancelOrder)
def cancel_order(ctx, event, location):
    order_id = event['data']['order_id']
    if order_id in ctx.orders:
        order_record = ctx.orders[order_id]
        ctx.logger.info('cancel account order %s', order_record['order'])
        ctx.commander.cancel_order(event, location.uid, order_id)
        return {
            'status': http.HTTPStatus.OK,
            'msg_type': msg.CancelOrder
        }
    else:
        return {
            'status': http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.CancelOrder
        }


@on(msg.CancelAllOrder)
def cancel_all_order(ctx, event, location):
    for order_id in ctx.orders:
        order_record = ctx.orders[order_id]
        if order_record['source'] == location.uid:
            ctx.logger.info('cancel account order %s', order_record['order'])
            ctx.commander.cancel_order(event, location.uid, order_id)
        if order_record['dest'] == location.uid:
            ctx.logger.info('cancel strategy order %s', order_record['order'])
            ctx.commander.cancel_order(event, location.uid, order_id)
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.CancelAllOrder
    }
