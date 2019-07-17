import pywingchun
import pyyjj
import json
import kungfu.yijinjing.time as kft
from kungfu.log import create_logger
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import to_dict
from kungfu.data.sqlite.data_proxy import make_url, DataProxy, LedgerHolder
from kungfu.finance.ledger import *
from kungfu.finance.position import *

DEFAULT_INIT_CASH = 1e7


class Watcher(pywingchun.Watcher):
    def __init__(self, ctx):
        mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
        pywingchun.Watcher.__init__(self, ctx.locator, mode, ctx.low_latency)
        self.ctx = ctx
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)

        self.trading_day = None

        location = pyyjj.location(mode, pyyjj.category.SYSTEM, 'watcher', 'watcher', ctx.locator)
        url = make_url(ctx.locator, location, ctx.name)
        self.data_proxy = DataProxy(url)
        self.ledger_holder = LedgerHolder(url)

        self.accounts = {}
        self.subportfolios = {}
        self.portfolios = {}

    def pre_start(self):
        self.add_time_interval(1 * kft.NANO_PER_MINUTE, lambda e: self.dump_snapshot())

    def print_trading_day(self, event):
        self.ctx.logger.info('timer print trading day %s', self.trading_day)

    def dump_snapshot(self, data_frequency = "minute"):
        messages = []
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            message = ledger.message
            message["data"]["update_time"] = self.now()
            message["msg_type"] = int(MsgType.AssetInfoSnapshot)
            message["data_frequency"] = data_frequency
            self.publish(json.dumps(message))
            messages.append(message)
        self.ledger_holder.on_messages(messages)

    def handle_request(self, msg):
        req = json.loads(msg)
        self.ctx.logger.debug("handle request %s", msg)
        self.ctx.logger.debug("handle request json %s", json.dumps(req))
        return json.dumps(req)

    def on_trading_day(self, event, daytime):
        self.ctx.logger.info('on trading day %s', kft.to_datetime(daytime))
        trading_day = kft.to_datetime(daytime)
        if self.trading_day is not None and self.trading_day != trading_day:
            self.dump_snapshot(data_frequency="daily")
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            ledger.apply_trading_day(trading_day)
        self.trading_day = trading_day

    def on_quote(self, event, quote):
        self.ctx.logger.info('on quote')
        for acc in self.accounts.values():
            acc.apply_quote(quote)
        for cli in self.portfolios.values():
            cli.apply_quote(quote)

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s from %s', order, self.get_location(event.dest).uname)
        order_dict = to_dict(order)
        order_dict["client_id"] = self.get_location(event.dest).name
        self.data_proxy.add_order(**order_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Order), "data": order_dict}))

    def on_trade(self, event, trade):
        self.ctx.logger.info('on trade %s from %s', trade, self.get_location(event.dest).uname)
        client_id = self.get_location(event.dest).name
        trade_dict = to_dict(trade)
        trade_dict["client_id"] = client_id
        self.data_proxy.add_trade(**trade_dict)
        self.publish(json.dumps({"msg_type": int(MsgType.Trade), "data": trade_dict}))
        if trade.account_id in self.accounts:
            self.accounts[trade.account_id].apply_trade(trade)
        self._get_portfolio_ledger(client_id).apply_trade(trade)

    def on_assets(self, asset_info, positions):
        self.ctx.logger.info("on assets, acc: %s", asset_info.account_id)
        for pos in positions:
            self.ctx.logger.info("on assets, pos: %s", pos)
        account = Ledger(trading_day = self.trading_day, ledger_category = asset_info.ledger_category, account_id = asset_info.account_id, source_id = asset_info.source_id, avail = asset_info.avail, positions = {get_symbol_id(pos.instrument_id, pos.exchange_id): StockPosition(**to_dict(pos)) for pos in positions})
        self.accounts[asset_info.account_id] = account
        account.register_callback(lambda messages: [ self.publish(json.dumps(message)) for message in messages])
        account.register_callback(self.ledger_holder.on_messages)

    def _get_portfolio_ledger(self, client_id):
        if not client_id in self.portfolios:
            ledger = self.ledger_holder.load(LedgerCategory.Portfolio, client_id = client_id)
            if ledger is None:
                ledger = Ledger(ledger_category = LedgerCategory.Portfolio, client_id = client_id, avail = DEFAULT_INIT_CASH, trading_day = self.trading_day)
            else:
                ledger.apply_trading_day(self.trading_day)
            ledger.register_callback(lambda messages: [ self.publish(json.dumps(message)) for message in messages])
            ledger.register_callback(self.ledger_holder.on_messages)
            self.portfolios[client_id] = ledger
        return self.portfolios[client_id]
