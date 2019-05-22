import os
import sys
import importlib
import ctypes
import nnpy
import json
from functools import partial
import pywingchun
import pyyjj

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
        self.on_quote = getattr(impl, 'on_quote', lambda ctx, quote: None)
        self.on_entrust = getattr(impl, 'on_entrust', lambda ctx, entrust: None)
        self.on_transaction = getattr(impl, "on_transaction", lambda ctx, transaction: None)
        self.on_order = getattr(impl, 'on_order', lambda ctx, order: None)
        self.on_trade = getattr(impl, 'on_trade', lambda ctx, trade: None)
        self.pre_run = getattr(impl, 'pre_run', lambda ctx: None)
        self.pre_quit = getattr(impl, 'pre_quit', lambda ctx: None)
        self.on_switch_day = getattr(impl, "on_switch_day", lambda ctx, trading_day: None)
        getattr(impl, 'init', lambda ctx: None)(self)

    def register_nanotime_callback(self, nano, callback):
        self._register_nanotime_callback(int(nano), partial(callback, self))
