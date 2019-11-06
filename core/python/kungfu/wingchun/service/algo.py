
import pywingchun
import json
import traceback
import sys
from kungfu.yijinjing.log import create_logger
from kungfu.wingchun.algo import AlgoOrder
from kungfu.wingchun.algo.algotypes import *
from kungfu.data.sqlite.data_proxy import AlgoDB

class Algo(pywingchun.AlgoService):
    def __init__(self, ctx):
        pywingchun.AlgoService.__init__(self, ctx.locator, ctx.mode, ctx.low_latency)
        self.ctx = ctx
        self.ctx.logger = create_logger("algo", ctx.log_level, self.io_device.home)
        self.ctx.db = AlgoDB(self.io_device.home, ctx.name)
        self.orders = {}

    def add_order(self, order):
        pywingchun.AlgoService.add_order(self, order)
        self.orders[order.order_id] = order

    def insert_order(self, event, msg):
        self.ctx.logger.info("{}".format(msg))
        try:
            input_params = json.loads(msg)
            type = input_params["algo_type"]
            order = AlgoOrder.create(type, **input_params)
            order.on_start(self.algo_context)
            self.add_order(order)
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def cancel_order(self, event, msg):
        try:
            pass
        except:
            pass

    def on_notice(self, order):
        source = order.source
        order_id = order.order_id
