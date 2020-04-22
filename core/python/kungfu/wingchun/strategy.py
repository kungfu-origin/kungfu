import os
import sys
import importlib
import kungfu.yijinjing.time as kft
from kungfu.wingchun import constants
from kungfu.wingchun import msg
from kungfu.wingchun import utils
from pykungfu import yijinjing as yjj
from pykungfu import wingchun as wc


class Runner(wc.Runner):
    def __init__(self, ctx, mode):
        wc.Runner.__init__(self, ctx.locator, ctx.group, ctx.name, mode, ctx.low_latency)
        self.ctx = ctx


class Strategy(wc.Strategy):
    def __init__(self, ctx):
        wc.Strategy.__init__(self)
        ctx.log = ctx.logger
        ctx.strftime = kft.strftime
        ctx.strptime = kft.strptime
        ctx.constants = constants
        ctx.utils = utils
        self.ctx = ctx
        self.ctx.books = {}
        self.__init_strategy(ctx.path)

    def __add_account(self, source, account, cash_limit):
        self.ctx.wc_context.add_account(source, account, cash_limit)

    def __get_account_book(self, source, account):
        location = yjj.location(yjj.mode.LIVE, yjj.category.TD, source, account, self.ctx.locator)
        return self.ctx.books[location.uid]

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
        self._on_bar = getattr(impl, "on_bar", lambda ctx, bar: None)
        self._on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self._on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self._on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self._on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self._on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)
        self._on_order_action_error = getattr(impl, 'on_order_action_error', lambda ctx, error: None)

    def __init_book(self):
        location = yjj.location(yjj.mode.LIVE, yjj.category.STRATEGY, self.ctx.group, self.ctx.name, self.ctx.locator)
        self.ctx.book = self.ctx.wc_context.bookkeeper.get_book(location.uid)

    def __add_timer(self, nanotime, callback):
        def wrap_callback(event):
            callback(self.ctx, event)

        self.ctx.wc_context.add_timer(nanotime, wrap_callback)

    def __add_time_interval(self, duration, callback):
        def wrap_callback(event):
            callback(self.ctx, event)

        self.ctx.wc_context.add_time_interval(duration, wrap_callback)

    def pre_start(self, wc_context):
        self.ctx.wc_context = wc_context
        self.ctx.now = wc_context.now
        self.ctx.add_timer = self.__add_timer
        self.ctx.add_time_interval = self.__add_time_interval
        self.ctx.subscribe = wc_context.subscribe
        self.ctx.subscribe_all = wc_context.subscribe_all
        self.ctx.add_account = self.__add_account
        self.ctx.list_accounts = wc_context.list_accounts
        self.ctx.get_account_cash_limit = wc_context.get_account_cash_limit
        self.ctx.insert_order = wc_context.insert_order
        self.ctx.cancel_order = wc_context.cancel_order
        self.ctx.is_book_held = wc_context.is_book_held
        self.ctx.is_positions_mirrored = wc_context.is_positions_mirrored
        self.ctx.hold_book = wc_context.hold_book
        self.ctx.hold_positions = wc_context.hold_positions
        self.ctx.get_account_book = self.__get_account_book
        self.__init_book()
        self._pre_start(self.ctx)

    def post_start(self, wc_context):
        self._post_start(self.ctx)

    def pre_stop(self, wc_context):
        self._pre_stop(self.ctx)

    def post_stop(self, wc_context):
        self._post_stop(self.ctx)

    def on_quote(self, wc_context, quote):
        self._on_quote(self.ctx, quote)

    def on_bar(self, wc_context, bar):
        self._on_bar(self.ctx, bar)

    def on_entrust(self, wc_context, entrust):
        self._on_entrust(self.ctx, entrust)

    def on_transaction(self, wc_context, transaction):
        self._on_transaction(self.ctx, transaction)

    def on_order(self, wc_context, order):
        self._on_order(self.ctx, order)

    def on_order_action_error(self, wc_context, error):
        self._on_order_action_error(self.ctx, error)

    def on_trade(self, wc_context, trade):
        self._on_trade(self.ctx, trade)

    def on_trading_day(self, wc_context, daytime):
        self.ctx.trading_day = kft.to_datetime(daytime)
        self._on_trading_day(self.ctx, daytime)
