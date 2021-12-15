import importlib
import json
import os
import sys
import types
import kungfu

from kungfu.console import site
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.practice.master import Master
from kungfu.yijinjing.practice.coloop import KungfuEventLoop

from collections import deque
from os import path

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing


class ExecutorRegistry:

    def __init__(self, ctx):
        self.ctx = ctx
        self.executors = {
            "system": {
                "master": MasterLoader(ctx),
                "service": ServiceLoader(ctx)
            },
            "md": {},
            "td": {},
            "strategy": {}
        }
        if ctx.extension_path:
            deque(map(self.register_extensions, ctx.extension_path.split(path.pathsep)))

    def register_extensions(self, root):
        for child in os.listdir(root):
            extension_dir = path.abspath(path.join(root, child))
            config_path = path.join(extension_dir, 'package.json')
            if path.exists(config_path):
                with open(config_path, "r") as config_file:
                    config = json.load(config_file)
                    if "kungfuConfig" in config:
                        group = config["kungfuConfig"]["key"]
                        for category in config["kungfuConfig"]["config"]:
                            if category not in kfj.CATEGORIES:
                                raise RuntimeError(f"Unsupported category {category}")
                            self.executors[category][group] = ExtensionLoader(self.ctx, extension_dir, config)

    def __getitem__(self, category):
        return self.executors[category]

    def __str__(self):
        return json.dumps(self.executors, indent=2, cls=RegistryJSONEncoder)

    def __repr__(self):
        return json.dumps(self.executors, cls=RegistryJSONEncoder)


class MasterLoader(dict):
    def __init__(self, ctx):
        super().__init__()
        self["master"] = lambda mode, low_latency: Master(ctx).run()


class ServiceLoader(dict):
    def __init__(self, ctx):
        super().__init__()
        self["ledger"] = lambda mode, low_latency: wc.Ledger(ctx.runtime_locator, kfj.MODES[ctx.mode], ctx.low_latency).run()


class ExtensionLoader:
    def __init__(self, ctx, extension_dir, config):
        self.ctx = ctx
        self.extension_dir = extension_dir
        self.config = config

    def __getitem__(self, name):
        return ExtensionExecutor(self.ctx, self)

    def __str__(self):
        return self.config["kungfuConfig"]["name"]

    def __repr__(self):
        return self.__str__()


class ExtensionExecutor:

    def __init__(self, ctx, loader):
        self.ctx = ctx
        self.loader = loader
        self.runners = {
            "md": self.run_market_data,
            "td": self.run_trader,
            "strategy": self.run_strategy
        }

    def __call__(self, mode, low_latency):
        self.runners[self.ctx.category]()

    def run_broker_vendor(self, vendor_builder):
        ctx = self.ctx
        loader = self.loader
        site.setup(loader.extension_dir)
        sys.path.insert(0, loader.extension_dir)
        module = importlib.import_module(ctx.group)
        vendor = vendor_builder(ctx.runtime_locator, ctx.group, ctx.name, ctx.low_latency)
        service = getattr(module, ctx.category)(vendor)
        vendor.setup(service)
        vendor.run()

    def run_market_data(self):
        self.run_broker_vendor(wc.MarketDataVendor)

    def run_trader(self):
        self.run_broker_vendor(wc.TraderVendor)

    def run_strategy(self):
        ctx = self.ctx
        loader = self.loader
        ctx.runner = wc.Runner(ctx, ctx.mode)
        ctx.runner.addStrategy(strategy)
        ctx.loop = KungfuEventLoop(ctx, ctx.runner)
        ctx.loop.run_forever()


class RegistryJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        test = isinstance(obj, ExtensionLoader) or isinstance(obj, types.FunctionType)
        return str(obj) if test else obj.__dict__
