
from kungfu.wingchun.algo import AlgoOrder
import json
from schema import Schema
import pywingchun
import sys
import traceback

class TwapOrder(AlgoOrder):
    __params_schema__ = Schema({"source_id": str, "account_id": str, "instrument_id": str, "exchange_id": str, "volume": int, "duration_ns": int, "interval_ns": int},ignore_extra_keys=True)
    __status_schema__ = Schema({"volume_traded": int, "volume": int, "volume_left": int}, ignore_extra_keys=True)

    def __init__(self, **kwargs):
        super(TwapOrder, self).__init__(**kwargs)
        params = kwargs.pop("params")
        self.params = TwapOrder.__params_schema__.validate(params)
        status = kwargs.pop("status", {"volume": self.params["volume"], "volume_traded": 0, "volume_left": self.params["volume"]})
        self.status = TwapOrder.__status_schema__.validate(status)

    def on_schedule(self, ctx, idx):
        order_input = pywingchun.OrderInput()
        order_input.source_id = self.params["source_id"]
        order_input.account_id = self.params["account_id"]
        order_input.instrument_id = self.params["instrument_id"]
        order_input.exchange_id = self.params["exchange_id"]
        order_input.volume = 100 #TODO
        order_input.parent_id = self.order_id
        order_id = ctx.insert_child_order(order_input)
        ctx.insert_child_order(order_input)
        self.send_notice()

    def on_start(self, ctx):
        tot_cnt = int(self.params["duration_ns"] / self.params["interval_ns"])
        for idx in range(tot_cnt):
            ctx.add_timer(ctx.now() + self.params["interval_ns"] * idx, lambda e: self.on_schedule(ctx, idx))
        ctx.add_timer(ctx.now() + self.params["duration_ns"], lambda e: self.on_stop(ctx))

    def on_stop(self, ctx):
        for order_id in self.active_orders:
            ctx.cancel_order(order_id)

    def on_child_order(self, ctx, order):
        pass

    def on_child_trade(self, ctx, trade):
        self.status["volume_traded"] += trade.volume
        self.status["volume_left"] -= trade.volume
        self.send_notice()

    def on_order_report(self, ctx, report_msg):
        print("on order report {}".format(report_msg))
        try:
            data = json.loads(report_msg)
            self.active = data["active"]
            self.status = TwapOrder.__status_schema__.validate(data["status"])
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            print('error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))



