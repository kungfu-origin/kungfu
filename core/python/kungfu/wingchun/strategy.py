import os, sys
import importlib
from functools import partial

import pywingchun
import kungfu.yijinjing.time as kft


class Strategy(pywingchun.Strategy):
    def __init__(self, ctx):
        pywingchun.Strategy.__init__(self)
        self.log = ctx.logger
        self.strftime = kft.strftime
        self.strptime = kft.strptime
        # context.is_subscribed = self._util.is_subscribed
        self.__init_strategy(ctx.path)

    def __init_strategy(self, path):
        strategy_dir = os.path.dirname(path)
        name_no_ext = os.path.split(os.path.basename(path))
        sys.path.append(os.path.relpath(strategy_dir))
        impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
        self._pre_start = getattr(impl, 'pre_start', lambda ctx: None)
        self._post_start = getattr(impl, 'post_start', lambda ctx: None)
        self._pre_stop = getattr(impl, 'pre_stop', lambda ctx: None)
        self._post_stop = getattr(impl, 'post_stop', lambda ctx: None)
        self._on_switch_day = getattr(impl, "on_switch_day", lambda ctx, trading_day: None)
        self._on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self._on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self._on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self._on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self._on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)

    def pre_start(self, ctx):
        self._pre_start(ctx)
        self.log.info('strategy prepare to run')

    def post_start(self, ctx):
        self._post_start(ctx)
        self.log.info('strategy ready to run')

    def pre_quit(self, ctx):
        self._pre_stop(ctx)

    def post_quit(self, ctx):
        self._post_stop(self, ctx)

    def on_switch_day(self, trading_day):
        self._on_switch_day(self, trading_day)

    def on_quote(self, quote):
        self._on_quote(self, quote)

    def on_entrust(self, entrust):
        self._on_entrust(self, entrust)

    def on_transaction(self, transaction):
        self._on_transaction(self, transaction)

    def on_order(self, order):
        self._on_order(self, order)

    def on_trade(self, trade):
        self._on_trade(self, trade)

