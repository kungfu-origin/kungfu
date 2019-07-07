import pywingchun
import pyyjj
import json
from kungfu.log import create_logger
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import to_dict
from kungfu.data.sqlite.data_proxy import make_url, DataProxy
from kungfu.finance.ledger import *
from kungfu.finance.position import *


class Watcher(pywingchun.Watcher):
    def __init__(self, ctx):
        mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
        pywingchun.Watcher.__init__(self, ctx.locator, mode, ctx.low_latency)
        self.ctx = ctx
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)
        location = pyyjj.location(mode, pyyjj.category.SYSTEM, 'watcher', 'watcher', ctx.locator)
        url = make_url(ctx.locator, location, ctx.name)
        self.data_proxy = DataProxy(url)
        self.accounts = {}

    def handle_request(self, msg):
        req = json.loads(msg)
        self.ctx.logger.debug("handle request %s", msg)
        self.ctx.logger.debug("handle request json %s", json.dumps(req))
        return json.dumps(req)

    def on_quote(self, event, quote):
        self.ctx.logger.info('on quote')
        return

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s from %s', order, self.get_location(event.dest).uname)
        order_dict = to_dict(order)
        self.data_proxy.add_order(**order_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Order), "data": order_dict}))

    def on_trade(self, event, trade):
        self.ctx.logger.info('on trade %s', trade)
        trade_dict = to_dict(trade)
        self.data_proxy.add_trade(**trade_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Trade), "data": trade_dict}))
        if trade.account_id in self.accounts:
            self.accounts[trade.account_id].apply_trade(trade)
            # self.publish(json.dumps({"msg_type": int(MsgType.AccountInfo),
            #                          "data": {**self.accounts[trade.account_id].message, **{"account_id":account_info.account_id, "source_id": "xtp"}}))
            # self.publish(json.dumps({}))

    def on_assets(self, account_info, positions):
        self.ctx.logger.info("on assets, acc: %s", account_info.account_id)
        for pos in positions:
            self.ctx.logger.info("on assets, pos: %s", pos)
        account = Ledger(avail = account_info.avail, positions = {get_symbol_id(pos.instrument_id, pos.exchange_id): StockPosition(**to_dict(pos)) for pos in positions})
        self.accounts[account_info.account_id] = account
        self.data_proxy.save_account(account_info.account_id, "xtp", account.message, [pos.message for pos in account.positions])
        self.publish(json.dumps({"msg_type": int(MsgType.AccountInfo), "data": {**{"account_id":account_info.account_id, "source_id": "xtp"}, **account.message}}))
        for pos in account.positions:
            self.publish(json.dumps({"msg_type": int(MsgType.Position), "data": {**{"account_id":account_info.account_id, "source_id": "xtp"}, **pos.message}}))
