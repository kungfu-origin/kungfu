import os
import sys
import importlib
import pyyjj
import pywingchun
from kungfu.wingchun.book.book import AccountBook
from kungfu.wingchun.constants import *
import kungfu.msg.utils as msg_utils
import kungfu.yijinjing.time as kft

class AlgoOrderContext:
    def __init__(self, wc_ctx):
        self._wc_ctx = wc_ctx
        self.orders = {}

    def insert_algo_order(self, order):
        order_id = self._wc_ctx.add_order(order)
        if order_id > 0:
            self.orders[order_id] = order

class Strategy(pywingchun.Strategy):
    def __init__(self, ctx):
        pywingchun.Strategy.__init__(self)
        ctx.log = ctx.logger
        ctx.strftime = kft.strftime
        ctx.strptime = kft.strptime
        ctx.inst_infos = {}
        self.ctx = ctx
        self.ctx.book = None
        self.ctx.books = {}
        self.__init_strategy(ctx.path)

    def __add_account(self, source, account, cash_limit):
        self.wc_context.add_account(source, account, cash_limit)
        location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, source, account, self.ctx.locator)
        book = AccountBook(self.ctx, location)
        self.ctx.books[location.uid] = book
        self.book_context.add_book(location, book)
        self.ctx.logger.info("added book {}@{}".format(account, source))

    def __get_account_book(self, source, account):
        location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, source, account, self.ctx.locator)
        return self.ctx.books[location.uid]

    def __get_inst_info(self, instrument_id):
        return msg_utils.object_as_dict(self.book_context.get_inst_info(instrument_id))

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
        location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.STRATEGY, self.ctx.group, self.ctx.name, self.ctx.locator)
        self.ctx.book = AccountBook(self.ctx, location)
        self.book_context.add_book(location, self.ctx.book)

    def __init_algo(self):
        class AlgoOrderContext:
            def __init__(self, wc_ctx):
                self._wc_ctx = wc_ctx
                self.orders = {}

            def insert_algo_order(self, order):
                order_id = self._wc_ctx.add_order(order)
                if order_id > 0:
                    self.orders[order_id] = order
        self.algo_context = AlgoOrderContext(self.algo_context)
        self.ctx.insert_algo_order = self.algo_context.insert_algo_order

    def __add_timer(self, nanotime, callback):
        def wrap_callback(event):
            callback(self.ctx, event)
        self.wc_context.add_timer(nanotime, wrap_callback)

    def __add_time_interval(self, duration, callback):
        def wrap_callback(event):
            callback(self.ctx, event)
        self.wc_context.add_time_interval(duration, wrap_callback)

    def pre_start(self, wc_context):
        self.ctx.logger.info("pre start")
        self.wc_context = wc_context
        self.book_context = wc_context.book_context
        self.algo_context = wc_context.algo_context
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
        self.ctx.get_account_book = self.__get_account_book
        self.ctx.get_inst_info = self.__get_inst_info
        self.__init_book()
        self.__init_algo()
        self._pre_start(self.ctx)
        self.ctx.log.info('strategy prepare to run')

    def post_start(self, wc_context):
        self._post_start(self.ctx)
        self.ctx.log.info('strategy ready to run')

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
        trading_day = kft.to_datetime(daytime)
        self.ctx.trading_day = trading_day
        self.ctx.logger.info("assign trading day {} for ctx".format(trading_day))
        self._on_trading_day(self.ctx, daytime)
