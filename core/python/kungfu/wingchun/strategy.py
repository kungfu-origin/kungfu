import os
import sys
import importlib
import pywingchun
import kungfu.yijinjing.time as kft


class Strategy(pywingchun.Strategy):
    def __init__(self, ctx):
        pywingchun.Strategy.__init__(self)
        ctx.log = ctx.logger
        ctx.strftime = kft.strftime
        ctx.strptime = kft.strptime
        self.ctx = ctx
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
        self._on_trading_day = getattr(impl, "on_trading_day", lambda ctx, trading_day: None)
        self._on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self._on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self._on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self._on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self._on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)

    def __add_timer(self, nanotime, callback):
        def wrap_callback(event):
            callback(self.ctx, event)
        self.wc_context.add_timer(nanotime, wrap_callback)

    def __add_time_interval(self, duration, callback):
        def wrap_callback(event):
            callback(self.ctx, event)
        self.wc_context.add_time_interval(duration, wrap_callback)

    def pre_start(self, wc_context):
        self.wc_context = wc_context
        self.ctx.now = wc_context.now
        self.ctx.add_timer = self.__add_timer
        self.ctx.add_time_interval = self.__add_time_interval
        self.ctx.subscribe = wc_context.subscribe
        self.ctx.add_account = wc_context.add_account
        self.ctx.insert_order = wc_context.insert_order
        self.ctx.insert_limit_order = wc_context.insert_limit_order
        self.ctx.insert_fok_order = wc_context.insert_fok_order
        self.ctx.insert_fak_order = wc_context.insert_fak_order
        self.ctx.insert_market_order = wc_context.insert_market_order
        self.ctx.cancel_order = wc_context.cancel_order
        self._pre_start(self.ctx)
        self.ctx.log.info('strategy prepare to run')

    def post_start(self, wc_context):
        self._post_start(self.ctx)
        self.ctx.log.info('strategy ready to run')

    def pre_quit(self, wc_context):
        self._pre_stop(self.ctx)

    def post_quit(self, wc_context):
        self._post_stop(self, self.ctx)

    def on_trading_day(self, wc_context, trading_day):
        self._on_trading_day(self.ctx, trading_day)

    def on_quote(self, wc_context, quote):
        self._on_quote(self.ctx, quote)

    def on_entrust(self, wc_context, entrust):
        self._on_entrust(self.ctx, entrust)

    def on_transaction(self, wc_context, transaction):
        self._on_transaction(self.ctx, transaction)

    def on_order(self, wc_context, order):
        self._on_order(self.ctx, order)

    def on_trade(self, wc_context, trade):
        self._on_trade(self.ctx, trade)

