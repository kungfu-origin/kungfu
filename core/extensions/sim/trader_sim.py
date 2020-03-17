from pykungfu import longfist as pylongfist
from pykungfu import yijinjing as yjj
from pykungfu import wingchun as pywingchun
import json
import kungfu.wingchun.utils as wc_utils
import kungfu.wingchun.msg as wc_msg
import os
import sys
import importlib
from dotted_dict import DottedDict
from collections import namedtuple


class MatchMode:
    Reject = "reject"
    Pend = "pend"
    Cancel = "cancel"
    PartialFillAndCancel = "partialfillandcancel"
    PartialFill = "partialfill"
    Fill = "fill"
    Custom = "custom"


OrderRecord = namedtuple("OrderRecord", ["source", "dest", "order"])


class TraderSim(pywingchun.Trader):
    def __init__(self, low_latency, locator, account_id, json_config):
        pywingchun.Trader.__init__(self, low_latency, locator, "sim", account_id)
        config = json.loads(json_config)
        self.match_mode = config.get("match_mode", MatchMode.Custom)

        self.ctx = DottedDict()
        self.ctx.orders = {}

        if self.match_mode == MatchMode.Custom:
            path = config.get("path")
            simulator_dir = os.path.dirname(path)
            name_no_ext = os.path.split(os.path.basename(path))
            sys.path.append(os.path.relpath(simulator_dir))
            impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
            self.ctx.insert_order = getattr(impl, 'insert_order', lambda ctx, event: False)
            self.ctx.cancel_order = getattr(impl, "cancel_order", lambda ctx, event: False)
            self.ctx.req_account = getattr(impl, "req_account", lambda ctx: False)
            self.ctx.req_position = getattr(impl, "req_position", lambda ctx: False)

    def on_start(self):
        pywingchun.Trader.on_start(self)

    def insert_order(self, event):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.insert_order(self.ctx, event)
        else:
            order_input = event.data
            order = pywingchun.utils.order_from_input(order_input)
            min_vol = 100 if wc_utils.get_instrument_type(order_input.instrument_id,
                                                          order_input.exchange_id) == pylongfist.enums.InstrumentType.Stock else 1
            if order_input.volume < min_vol:
                order.status = pylongfist.enums.OrderStatus.Error
            elif self.match_mode == MatchMode.Reject:
                order.status = pylongfist.enums.OrderStatus.Error
            elif self.match_mode == MatchMode.Pend:
                order.status = pylongfist.enums.OrderStatus.Pending
            elif self.match_mode == MatchMode.Cancel:
                order.status = pylongfist.enums.OrderStatus.Cancelled
            elif self.match_mode == MatchMode.PartialFillAndCancel:
                order.volume_traded = min_vol
                order.status = pylongfist.enums.OrderStatus.Filled if order.volume_traded == order.volume \
                    else pylongfist.enums.OrderStatus.PartialFilledNotActive
            elif self.match_mode == MatchMode.PartialFill:
                order.volume_traded = min_vol
                order.status = pylongfist.enums.OrderStatus.Filled if order.volume_traded == order.volume \
                    else pylongfist.enums.OrderStatus.PartialFilledActive
            elif self.match_mode == MatchMode.Fill:
                order.volume_traded = order_input.volume
            else:
                raise Exception("invalid match mode {}".format(self.match_mode))
            order.volume_left = order.volume - order.volume_traded
            self.get_writer(event.source).write_data(0, wc_msg.Order, order)
            if order.volume_traded > 0:
                trade = pywingchun.Trade()
                trade.account_id = self.io_device.home.name
                trade.order_id = order.order_id
                trade.volume = order.volume_traded
                trade.price = order.limit_price
                trade.instrument_id = order.instrument_id
                trade.exchange_id = order.exchange_id
                trade.trade_id = self.get_writer(event.source).current_frame_uid()
                self.get_writer(event.source).write_data(0, wc_msg.Trade, trade)
            if order.active:
                self.ctx.orders[order.order_id] = OrderRecord(source=event.source, dest=event.dest, order=order)
            return True

    def cancel_order(self, event):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.cancel_order(self.ctx, event)
        else:
            order_action = event.data
            if order_action.order_id in self.ctx.orders:
                record = self.ctx.orders.pop(order_action.order_id)
                order = record.order
                order.status = pylongfist.enums.OrderStatus.Cancelled if order.volume_traded == 0 \
                    else pylongfist.enums.OrderStatus.PartialFilledNotActive
                self.get_writer(event.source).write_data(0, wc_msg.Order, order)
            return True

    def re_account(self):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.req_account(self.ctx)
        return False

    def req_position(self):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.req_position(self.ctx)
        return False
