import pywingchun
import json
from kungfu.log import create_logger


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
        return

    def on_trade(self, event, trade):
        self.ctx.logger.info('on trade')
        return
