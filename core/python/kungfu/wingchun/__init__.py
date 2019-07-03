import pywingchun
import json
from kungfu.log import create_logger
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import to_dict

class Watcher(pywingchun.Watcher):
    def __init__(self, ctx):
        pywingchun.Watcher.__init__(self, ctx.low_latency, ctx.locator)
        self.ctx = ctx
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)

    def handle_request(self, msg):
        req = json.loads(msg)
        self.ctx.logger.debug("handle request %s", msg)
        self.ctx.logger.debug("handle request json %s", json.dumps(req))
        return json.dumps(req)

    def on_quote(self, event, quote):
        self.ctx.logger.info('on quote')
        return

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s', order)
        order_dict = to_dict(order)
        self.ctx.data_proxy.add_order(**order_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Order), "data": order_dict}))

    def on_trade(self, event, trade):
        self.ctx.logger.info('on trade %s', trade)
        trade_dict = to_dict(trade)
        self.ctx.data_proxy.add_trade(**trade_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Trade), "data": trade_dict}))
