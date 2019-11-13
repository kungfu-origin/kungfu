
import pywingchun
import json
import traceback
import sys
import kungfu.wingchun.msg as wc_msg
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
        self.orders[order.order_id] = order
        pywingchun.AlgoService.add_order(self, order)
        order.subject.subscribe(self.on_notice)

    def insert_order(self, event, msg):
        self.ctx.logger.info("rcv insert order msg {} from {}[{:08x}]".format(msg, self.get_location(event.source).uname, event.source))
        try:
            sender_uid = event.source
            args = json.loads(msg)
            type = args["algo_type"]
            args["sender_uid"] = sender_uid
            order = AlgoOrder.create(type, **args)
            order.on_start(self.algo_context)
            self.add_order(order)
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def cancel_order(self, event, order_action):
        self.ctx.logger.debug("rcv cancel order msg {} from {}[{:08x}]".format(order_action, self.get_location(event.source).uname, event.source))
        try:
            order_id = order_action.order_id
            if order_id in self.orders:
                order = self.orders[order_id]
                order.on_stop(self.algo_context)
            else:
                self.ctx.warn("failed to find order {}".format(order_id))
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def modify_order(self, event, msg):
        self.ctx.logger.debug("rcv modify order msg {} from {}[{:08x}]".format(msg, self.get_location(event.source), event.source))
        try:
            data = json.loads(msg)
            order_id = data["order_id"]
            if order_id in self.orders:
                order = self.orders[order_id]
                order.on_modify(self.algo_context, data)
            else:
                self.ctx.warn("failed to find order {}".format(order_id))
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def on_notice(self, order):
        self.ctx.logger.debug(order)
        if self.has_writer(order.sender_uid):
            writer = self.get_writer(order.sender_uid)
            report_msg = order.dumps()
            self.ctx.logger.info("write {}".format(report_msg))
            writer.write_str(0, wc_msg.AlgoOrderReport, report_msg)
        else:
            if self.has_location(order.sender_uid):
                location = self.get_location(order.sender_uid)
                self.ctx.logger.error("has no writer for {}[{:08x}]".format(location.uname, location.uid))
            else:
                self.ctx.logger.error("has no writer for {}[{:08x}]".format("unknown", order.sender_uid))
        self.ctx.db.add_order(order_id = order.order_id,
                              sender_uid = order.sender_uid,
                              algo_type = order.type,
                              update_time = self.now(),
                              params = order.params,
                              status = order.status,
                              active = order.active)


