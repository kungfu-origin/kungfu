import os, sys
import importlib
from functools import partial

import pywingchun
import nnpy, pyyjj
import kungfu.wingchun.constants

from kungfu.command import command, arg

@arg('-p', '--path', type=str, help='path of strategy py file')
@command(help='run trading strategy')
def strategy(args, logger):
    Strategy(logger, args.name, args.path).run()

class Strategy(pywingchun.Strategy):
    def __init__(self, logger, name, path):
        pywingchun.Strategy.__init__(self, name)
        self.log = logger
        self.parse_nano = pyyjj.parse_nano
        self.parse_time = pyyjj.parse_time
        # context.is_subscribed = self._util.is_subscribed
        self.__init_strategy(path)

    def __init_strategy(self, path):
        strategy_dir = os.path.dirname(path)
        name_no_ext = os.path.split(os.path.basename(path))
        sys.path.append(os.path.relpath(strategy_dir))
        impl = importlib.import_module(os.path.splitext(name_no_ext[1])[0])
        self._pre_run = getattr(impl, 'pre_run', lambda ctx: None)
        self._pre_quit = getattr(impl, 'pre_quit', lambda ctx: None)
        self._on_switch_day = getattr(impl, "on_switch_day", lambda ctx, trading_day: None)
        self._on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self._on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self._on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self._on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self._on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)
        getattr(impl, 'init', lambda ctx: None)(self)

    def register_nanotime_callback(self, nano, callback):
        self._register_nanotime_callback(int(nano), partial(callback, self))
    
    def pre_run(self):
        self._pre_run(self)
    
    def pre_quit(self):
        self._pre_quit(self)
    
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
