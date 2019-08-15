import pywingchun
import pyyjj
import json
from itertools import groupby
import kungfu.yijinjing.time as kft
from kungfu.yijinjing.log import create_logger
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import object_as_dict
from kungfu.data.sqlite.data_proxy import make_url, DataProxy, LedgerHolder
from kungfu.wingchun.finance.ledger import *
from kungfu.wingchun.finance.position import *

DEFAULT_INIT_CASH = 1e7


class Watcher(pywingchun.Watcher):
    def __init__(self, ctx):
        mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
        pywingchun.Watcher.__init__(self, ctx.locator, mode, ctx.low_latency)        
        self.ctx = ctx
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)
        self.trading_day = None
        location = pyyjj.location(mode, pyyjj.category.SYSTEM, 'watcher', 'watcher', ctx.locator)
        DataProxy.setup_logger(ctx.log_level, self.io_device.home)
        url = make_url(ctx.locator, location, ctx.name)
        data_proxy = DataProxy(url)
        self.ctx.data_proxy = data_proxy
        self.ctx.ledger_holder = LedgerHolder(url)

        self.accounts = {}  
        self.subportfolios = {}
        self.portfolios = {}

    def pre_start(self):
        self.add_time_interval(1 * kft.NANO_PER_MINUTE, lambda e: self._dump_snapshot())

    def handle_request(self, msg):
        req = json.loads(msg)
        self.ctx.logger.debug("handle request %s", msg)
        self.ctx.logger.debug("handle request json %s", json.dumps(req))
        return json.dumps(req)

    def on_trading_day(self, event, daytime):
        self.ctx.logger.info('on trading day %s', kft.to_datetime(daytime))
        trading_day = kft.to_datetime(daytime)
        if self.trading_day is not None and self.trading_day != trading_day:
            self._switch_day()
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            ledger.apply_trading_day(trading_day)
        self.trading_day = trading_day

    def on_quote(self, event, quote):
        self.ctx.logger.info('on quote')
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            ledger.apply_quote(quote)

    def on_order(self, event, order):
        self.ctx.logger.info('on order %s from %s', order, self.get_location(event.dest).uname)
        message = self._message_from_order_event(event, order)
        self.ctx.data_proxy.add_order(**message["data"])
        self.publish(json.dumps(message))

    def on_trade(self, event, trade):
        self.ctx.logger.info('on trade %s from %s', trade, self.get_location(event.dest).uname)
        client_id = self.get_location(event.dest).name
        message = self._message_from_trade_event(event, trade)
        self.ctx.data_proxy.add_trade(**message["data"])
        self.publish(json.dumps(message))
        self._get_account_ledger(account_id=trade.account_id).apply_trade(trade)
        self._get_portfolio_ledger(client_id).apply_trade(trade)

    def on_instruments(self, instruments):               
        if list(set(instruments)):            
            self.ctx.data_proxy.set_instruments([object_as_dict(inst) for inst in list(set(instruments))])

    def on_stock_account(self, asset, positions):
        pos_objects = [StockPosition(**object_as_dict(pos)) for pos in positions]
        account = Ledger(ctx = self.ctx,
                         trading_day = self.trading_day,
                         ledger_category = LedgerCategory.Account,
                         account_id = asset.account_id,
                         source_id = asset.source_id,
                         avail = asset.avail,
                         positions = pos_objects)
        self._reset_account_ledger(asset.account_id, account)

    def on_future_account(self, asset, position_details):
        pos_objects = []
        for inst, details in groupby(position_details, key= lambda e: e.instrument_id):
            instrument_info = self.ctx.data_proxy.get_instrument_info(inst)
            detail_objects = []
            for detail in sorted(details, key=lambda detail: (detail.open_date, detail.trade_time)):
                args = object_as_dict(detail)
                args.update({"contract_multiplier":instrument_info["contract_multiplier"],
                             "long_margin_ratio": instrument_info["long_margin_ratio"],
                             "short_margin_ratio": instrument_info["short_margin_ratio"]})
                detail_objects.append(FuturePositionDetail(**args))
            pos_args = {"details": detail_objects,"trading_day":self.trading_day}
            pos_args.update(instrument_info)
            pos = FuturePosition(**pos_args)
            pos_objects.append(pos)
        account = Ledger(ctx = self.ctx,
                         trading_day = self.trading_day,
                         ledger_category = LedgerCategory.Account,
                         account_id = asset.account_id,
                         source_id = asset.source_id,
                         avail = asset.avail,
                         positions = pos_objects)
        self._reset_account_ledger(asset.account_id, account)

    def _message_from_order_event(self, event, order):
        order_dict = object_as_dict(order)
        order_dict["order_id"] = str(order.order_id)
        order_dict["parent_id"] = str(order.parent_id)
        order_dict["client_id"] = self.get_location(event.dest).name
        return {"msg_type": event.msg_type,"data": order_dict}

    def _message_from_trade_event(self, event, trade):
        client_id = self.get_location(event.dest).name
        trade_dict = object_as_dict(trade)
        trade_dict["order_id"] = str(trade.order_id)
        trade_dict["parent_order_id"] = str(trade.parent_order_id)
        trade_dict["trade_id"] = str(trade.trade_id)
        trade_dict["client_id"] = client_id
        return {"msg_type": event.msg_type,"data": trade_dict}

    def _dump_snapshot(self, data_frequency="minute"):
        messages = []
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            message = ledger.asset_message
            message["msg_type"] = int(MsgType.AssetSnapshot)
            tags = {"update_time": self.now(), "data_frequency": data_frequency}
            message["data"].update(tags)
            self.publish(json.dumps(message))
            messages.append(message)
        self.ctx.ledger_holder.on_messages(messages)

    def _switch_day(self):
        self._dump_snapshot(data_frequency="daily")
        for ledger in list(self.accounts.values()) + list(self.portfolios.values()):
            self.ctx.ledger_holder.dump(ledger)

    def _reset_account_ledger(self, account_id, ledger):
        self.ctx.logger.info("reset account ledger, {}".format(account_id))
        account = self._get_account_ledger(account_id)
        account.merge(ledger)
        self.ctx.ledger_holder.dump(account)

    def _get_account_ledger(self, account_id):
        return self._get_ledger(ledger_category=LedgerCategory.Account,account_id=account_id, create_if_not_existing=True)

    def _get_portfolio_ledger(self, client_id):
       return self._get_ledger(ledger_category=LedgerCategory.Portfolio, client_id=client_id,create_if_not_existing=True)

    def _get_ledger(self, ledger_category, account_id = "", client_id = "", create_if_not_existing=False):
        if ledger_category == LedgerCategory.Account:
            ledger_dict = self.accounts
            ledger_key = account_id
        elif ledger_category ==  LedgerCategory.Portfolio:
            ledger_dict = self.portfolios
            ledger_key = client_id
        else:
            raise ValueError("Unrecognized ledger category {}".format(ledger_category))
        if ledger_key in ledger_dict:
            return ledger_dict[ledger_key]
        else:
            ledger = self.ctx.ledger_holder.load(ledger_category=ledger_category, account_id=account_id, client_id=client_id)
            if not ledger and create_if_not_existing:
                ledger = Ledger(self.ctx, ledger_category = ledger_category,account_id=account_id,client_id = client_id, avail = DEFAULT_INIT_CASH, trading_day = self.trading_day)
            if ledger:
                ledger._ctx = self.ctx
                ledger.apply_trading_day(self.trading_day)
                ledger.register_callback(lambda messages: [ self.publish(json.dumps(message)) for message in messages])
                ledger.register_callback(self.ctx.ledger_holder.on_messages)
                ledger_dict[ledger_key] = ledger
            return ledger
