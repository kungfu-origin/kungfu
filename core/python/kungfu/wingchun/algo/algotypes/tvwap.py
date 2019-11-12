
from kungfu.wingchun.algo import AlgoOrder
import json
from schema import Schema
import pywingchun

class TwapOrder(AlgoOrder):
    __params_schema__ = Schema({"source_id": str, "account_id": str, "instrument_id": str, "exchange_id": str, "volume": int, "duration": int, "interval": int},ignore_extra_keys=True)
    __status_schema__ = Schema({"volume_traded": int, "volume": int, "volume_left": int, "tot_order_count", int, "cur_order_count": int}, ignore_extra_keys=True)

    def __init__(self, ctx, **kwargs):
        super(TwapOrder, self).__init__(ctx, **kwargs)
        params = kwargs.pop("params")
        self.params = TwapOrder.__params_schema__.validate(params)
        status = kwargs.pop("status", {"volume_traded": 0, "volume": self.params["volume"], "volume_left": self.params["volume"],
                                       "tot_order_count": int(self.params["duration"]/ self.params["interval", "cur_order_count": 0])})
        self.status = TwapOrder.__status_schema__.validate(status)

    def insert_child_order(self, ctx):
        order_input = pywingchun.OrderInput()
        order_input.source_id = self.params["source_id"]
        order_input.account_id = self.params["account_id"]
        order_input.instrument_id = self.params["instrument_id"]
        order_input.exchange_id = self.params["exchange_id"]
        order_input.volume = 100
        order_input.parent_id = self.order_id
        order_id = ctx.insert_child_order(order_input)
        self.ctx.logger.debug("{} {} insert child order {}[{}]".format(self.__class__.__name__, self.order_id, order_id, order_input)
        self.status["cur_order_count"] += 1
        return order_id

    def on_start(self, ctx):
        for idx in range(self.tot_order_count):
            self,ctx.add_timer(ctx.now() + self.params["interval"] * idx, lambda e: self.insert_child_order(ctx))
        self.ctx.debug("{} {} started".format(self.__class__.__name__, self.order_id))

    def on_child_order(self, ctx, order):
        self.ctx.debug("{} {} child order received {}".format(self.__class__.__name__, self.order_id, order))

    def on_child_trade(self, ctx, trade):
        self.ctx.debug("{} {} child trade received {}".format(self.__class__.__name__, self.order_id, trade))

    def on_order_status(self, ctx, status):
        try:
            status = json.loads(status)
            self.status = TwapOrder.__status_schema__.validate(status)
        except:
            pass




