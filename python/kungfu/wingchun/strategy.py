import os
import sys
import importlib
import ctypes
import nnpy
import json
from functools import partial
from kungfu.wingchun.structs import *
from kungfu.wingchun.EventLoop import EventLoop
from kungfu.wingchun.context import context
from kungfu.wingchun.constants import *
import pystrategy
import pyyjj

class Strategy:
    def __init__(self, name, path):
        self._base_dir = os.environ['KF_HOME']

        self._name = name
        self._event_loop = EventLoop(name)
        self._util = pystrategy.Util(name)

        self._on_quote = None
        self._on_order = None
        self._on_trade = None
        self.__init_context()
        self.__init_strategy(path)

    def __init_context(self):

        context.stop = self.stop

        context.add_md = self.add_md
        context.add_account = self.add_account
        context.register_algo_service = self.register_algo_service

        context.subscribe = self._util.subscribe
        context.is_subscribed = self._util.is_subscribed

        context.insert_limit_order = self._util.insert_limit_order
        context.insert_fok_order = self._util.insert_fok_order
        context.insert_fak_order = self._util.insert_fak_order
        context.insert_market_order = self._util.insert_market_order
        context.cancel_order = self._util.cancel_order

        context.set_log_level = self._util.set_log_level
        context.log_info = self._util.log_info
        context.log_error = self._util.log_error
        context.log_warn = self._util.log_warn

        context.get_last_md = self.get_last_md
        context.get_position = self.get_position
        context.get_portfolio_info = self.get_portfolio_info
        context.get_sub_portfolio_info = self.get_sub_portfolio_info

        context.get_nano = self.get_nano
        context.register_nanotime_callback = self.register_nanotime_callback
        context.register_nanotime_callback_with_context = self.register_nanotime_callback_with_context
        context.parse_nano = pyyjj.parse_nano
        context.parse_time = pyyjj.parse_time

    def __init_strategy(self, path):
        strategy_dir = os.path.dirname(path)
        name_no_ext = os.path.split(os.path.basename(path))
        sys.path.append(os.path.relpath(strategy_dir))
        impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
        self._on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self._on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self._on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self._on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self._on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)
        self._pre_run = getattr(impl, 'pre_run', lambda ctx: None)
        self._pre_quit = getattr(impl, 'pre_quit', lambda ctx: None)
        self._on_switch_day = getattr(impl, "on_switch_day", lambda ctx, trading_day: None)
        self._util.register_switch_day_callback(lambda trading_day: self._on_switch_day(context, trading_day))

        getattr(impl, 'init', lambda ctx: None)(context)

    def run(self):

        self._event_loop.register_nanotime_callback(self.__nseconds_next_min(pyyjj.nano_time()), self.__on_1min_timer)
        self._event_loop.register_nanotime_callback(self.__nseconds_next_day(pyyjj.nano_time()), self.__on_1day_timer)

        self._event_loop.register_quote_callback(self.__process_quote)
        self._event_loop.register_entrust_callback(self.__process_entrust)
        self._event_loop.register_transaction_callback(self.__process_transaction)
        self._event_loop.register_order_callback(self.__process_order)
        self._event_loop.register_trade_callback(self.__process_trade)

        self._event_loop.register_signal_callback(lambda sig: self._pre_quit(context))

        self._pre_run(context)
        self._event_loop.run()

    def stop(self):
        self._event_loop.stop()
        self._pre_quit(context)

    def add_md(self, source_id):
        self._event_loop.subscribe_yjj_journal(self.__get_md_journal_folder(source_id), self.__get_md_journal_name(source_id), pyyjj.nano_time())
        return self._util.add_md(source_id)

    def add_account(self, source_id, account_id, cash_limit):
        self._event_loop.subscribe_yjj_journal(self.__get_td_journal_folder(source_id, account_id), self.__get_td_journal_name(source_id, account_id), pyyjj.nano_time())
        self._event_loop.register_reload_instruments_callback(self._util.reload_instruments)
        url = 'ipc://' + self._base_dir + '/gateway/td_' + source_id + '_' + account_id + '/pub.ipc'
        self._event_loop.subscribe_nanomsg(url)
        return self._util.add_account(source_id, account_id, cash_limit)

    def register_algo_service(self):
        return self._util.register_algo_service()

    def register_nanotime_callback(self, nano, callback):
        self._event_loop.register_nanotime_callback(nano, callback)

    def register_nanotime_callback_with_context(self, nano, callback):
        self._event_loop.register_nanotime_callback(nano, partial(callback, context))

    def get_nano(self):
        return self._event_loop.get_nano()

    def get_last_md(self, instrument_id, exchange_id):
        quote_ptr = pystrategy.get_last_md(self._util, instrument_id, exchange_id)
        ctypes_quote = ctypes.cast(quote_ptr, ctypes.POINTER(Quote)).contents
        pystrategy.release_ptr(quote_ptr)
        return ctypes_quote

    def get_position(self, instrument_id, exchange_id, direction = Direction.Long, account_id = ""):
        pos_ptr = pystrategy.get_position(self._util, instrument_id, exchange_id, direction, account_id)
        ctypes_pos = ctypes.cast(pos_ptr, ctypes.POINTER(Position)).contents
        pystrategy.release_ptr(pos_ptr)
        return ctypes_pos

    def get_portfolio_info(self):
        pnl_ptr = pystrategy.get_portfolio_info(self._util)
        ctypes_pnl = ctypes.cast(pnl_ptr, ctypes.POINTER(PortfolioInfo)).contents
        pystrategy.release_ptr(pnl_ptr)
        return ctypes_pnl

    def get_sub_portfolio_info(self, account_id):
        sub_pnl_ptr = pystrategy.get_sub_portfolio_info(self._util, account_id)
        ctypes_sub_pnl = ctypes.cast(sub_pnl_ptr, ctypes.POINTER(SubPortfolioInfo)).contents
        pystrategy.release_ptr(sub_pnl_ptr)
        return ctypes_sub_pnl

    def __on_1min_timer(self, nano):
        self._util.on_push_by_min()
        self._event_loop.register_nanotime_callback(nano + 60 * 1000000000, self.__on_1min_timer)

    def __on_1day_timer(self, nano):
        self._util.on_push_by_day()
        self._event_loop.register_nanotime_callback(nano + 24 * 60 * 60 * 1000000000, self.__on_1day_timer)

    def __get_md_journal_folder(self, source):
        return self._base_dir + '/journal/md/' + source

    def __get_md_journal_name(self, source):
        return "md_" + source

    def __get_td_journal_folder(self, source, account_id):
        return self._base_dir + '/journal/td/' + source + '/' + account_id

    def __get_td_journal_name(self, source, account_id):
        return "td_{}_{}".format(source, account_id)

    def __nseconds_next_min(self, nano):
        cur_time = pyyjj.parse_nano(nano, '%Y%m%d %H:%M:%S')
        return pyyjj.parse_time(cur_time[:-2] + '00', '%Y%m%d %H:%M:%S') + 60 * 1e9

    def __nseconds_next_day(self, nano):
        cur_time = pyyjj.parse_nano(nano, '%Y%m%d %H:%M:%S')
        next_day = pyyjj.parse_time(cur_time[0:9] + '15:30:00', '%Y%m%d %H:%M:%S')
        if next_day < pyyjj.nano_time():
            next_day += 24 * 60 * 60 * 1000000000
        return next_day

    def __process_quote(self, quote):
        ctype_quote = ctypes.cast(quote,ctypes.POINTER(Quote)).contents
        if self._util.is_subscribed(ctype_quote.source_id, ctype_quote.instrument_id, ctype_quote.exchange_id):
            self._util.on_quote(quote)
            self._on_quote(context, ctype_quote)

    def __process_entrust(self, entrust):
        ctype_entrust = ctypes.cast(entrust,ctypes.POINTER(Entrust)).contents
        if self._util.is_subscribed(ctype_entrust.source_id, ctype_entrust.instrument_id, ctype_entrust.exchange_id):
            self._on_entrust(context, ctype_entrust)

    def __process_transaction(self, transaction):
        ctype_transaction = ctypes.cast(transaction,ctypes.POINTER(Transaction)).contents
        if self._util.is_subscribed(ctype_transaction.source_id, ctype_transaction.instrument_id, ctype_transaction.exchange_id):
            self._on_transaction(context, ctype_transaction)

    def __process_order(self, order):
        ctype_order = ctypes.cast(order,ctypes.POINTER(Order)).contents
        if self._name == ctype_order.client_id.decode('utf-8'):
            self._util.on_order(order)
            self._on_order(context, ctype_order)

    def __process_trade(self, trade):
        ctype_trade = ctypes.cast(trade,ctypes.POINTER(Trade)).contents
        if self._name == ctype_trade.client_id.decode('utf-8'):
            self._util.on_trade(trade)
            self._on_trade(context, ctype_trade)
