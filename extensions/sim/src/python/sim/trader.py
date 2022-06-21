import importlib
import json
import kungfu
import os
import sys

from dotted_dict import DottedDict
from collections import namedtuple
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing import time as kft

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing


class MatchMode:
    Reject = "reject"
    Pend = "pend"
    Cancel = "cancel"
    PartialFillAndCancel = "partialfillandcancel"
    PartialFill = "partialfill"
    Fill = "fill"
    Custom = "custom"


OrderRecord = namedtuple("OrderRecord", ["source", "dest", "order"])


class TraderSim(wc.Trader):
    def __init__(self, vendor):
        wc.Trader.__init__(self, vendor)
        self.logger = create_logger(
            "_".join(('td', self.home.group, self.home.name)),
            "info",
            yjj.location(
                lf.enums.mode.LIVE,
                lf.enums.category.TD,
                self.home.group,
                self.home.name,
                vendor.home.locator,
            ),
        )

    def on_start(self):
        config = json.loads(self.config)
        self.match_mode = config.get("match_mode", MatchMode.Custom)

        self.ctx = DottedDict()
        self.ctx.orders = {}

        if self.match_mode == MatchMode.Custom:
            path = config.get("path")
            simulator_dir = os.path.dirname(path)
            name_no_ext = os.path.split(os.path.basename(path))
            sys.path.append(os.path.relpath(simulator_dir))
            impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
            self.ctx.insert_order = getattr(
                impl, "insert_order", lambda ctx, event: False
            )
            self.ctx.cancel_order = getattr(
                impl, "cancel_order", lambda ctx, event: False
            )
            self.ctx.req_account = getattr(impl, "req_account", lambda ctx: False)
            self.ctx.req_position = getattr(impl, "req_position", lambda ctx: False)

        self.update_broker_state(lf.enums.BrokerState.Ready)

    def insert_order(self, event):
        volume_traded = 0

        if self.match_mode == MatchMode.Custom:
            return self.ctx.insert_order(self.ctx, event)
        else:
            writer = self.get_writer(event.source)
            order_input = event.OrderInput()
            order = wc.utils.order_from_input(order_input)
            order.insert_time = event.gen_time
            order.update_time = event.gen_time
            order.trading_day = kft.strfnow("%Y%m%d")
            min_vol = (
                100
                if wc.utils.get_instrument_type(
                    order_input.exchange_id, order_input.instrument_id
                )
                == lf.enums.InstrumentType.Stock
                else 1
            )
            if order_input.volume < min_vol:
                order.status = lf.enums.OrderStatus.Error
            elif self.match_mode == MatchMode.Reject:
                order.status = lf.enums.OrderStatus.Error
            elif self.match_mode == MatchMode.Pend:
                order.status = lf.enums.OrderStatus.Pending
            elif self.match_mode == MatchMode.Cancel:
                order.status = lf.enums.OrderStatus.Cancelled
            elif self.match_mode == MatchMode.PartialFillAndCancel:
                volume_traded = min_vol
                order.status = (
                    lf.enums.OrderStatus.Filled
                    if volume_traded == order.volume
                    else lf.enums.OrderStatus.PartialFilledNotActive
                )
            elif self.match_mode == MatchMode.PartialFill:
                volume_traded = min_vol
                order.status = (
                    lf.enums.OrderStatus.Filled
                    if volume_traded == order.volume
                    else lf.enums.OrderStatus.PartialFilledActive
                )
            elif self.match_mode == MatchMode.Fill:
                volume_traded = order_input.volume
                order.status = lf.enums.OrderStatus.Filled
            else:
                raise Exception("invalid match mode {}".format(self.match_mode))
            order.volume_left = order.volume - volume_traded
            writer.write(event.gen_time, order)
            self.ctx.orders[order.order_id] = order

            if volume_traded > 0:
                trade = lf.types.Trade()
                trade.trade_id = writer.current_frame_uid()
                trade.source_id = self.home.group
                trade.account_id = self.home.name
                trade.order_id = order.order_id
                trade.volume = volume_traded
                trade.price = order.limit_price
                trade.side = order.side
                trade.offset = order.offset
                trade.instrument_id = order.instrument_id
                trade.instrument_type = order.instrument_type
                trade.exchange_id = order.exchange_id
                trade.trade_time = yjj.now_in_nano()
                writer.write(event.gen_time, trade)

            return True

    def cancel_order(self, event):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.cancel_order(self.ctx, event)
        else:
            writer = self.get_writer(event.source)
            order_action = event.OrderAction()
            if order_action.order_id in self.ctx.orders:
                order = self.ctx.orders.pop(order_action.order_id)
                order.update_time = yjj.now_in_nano()
                order.status = (
                    lf.enums.OrderStatus.Cancelled
                    if order.volume - order.volume_left == 0
                    else lf.enums.OrderStatus.PartialFilledNotActive
                )
                writer.write(event.gen_time, order)
            return True

    def req_account(self):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.req_account(self.ctx)
        return False

    def req_position(self):
        if self.match_mode == MatchMode.Custom:
            return self.ctx.req_position(self.ctx)
        return False
