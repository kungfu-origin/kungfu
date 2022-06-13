import importlib
import json
import os
import sys
import types
import kungfu

from kungfu.console import site
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.practice.master import Master
from kungfu.yijinjing.practice.coloop import KungfuEventLoop
from kungfu.wingchun.strategy import Runner, Strategy

from collections import deque
from importlib.util import module_from_spec, spec_from_file_location
from os import path

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing


class ExecutorRegistry:
    def __init__(self, ctx):
        self.ctx = ctx
        self.executors = {
            "system": {"master": MasterLoader(ctx), "service": ServiceLoader(ctx)},
            "md": {},
            "td": {},
            "strategy": {"default": ExtensionLoader(self.ctx, None, None)},
        }
        if ctx.extension_path:
            deque(map(self.register_extensions, ctx.extension_path.split(path.pathsep)))
        elif ctx.path:
            self.read_config(os.path.join(os.path.dirname(ctx.path), "package.json"))

    def register_extensions(self, root):
        for child in os.listdir(root):
            extension_dir = path.abspath(path.join(root, child))
            config_path = path.join(extension_dir, "package.json")
            self.read_config(config_path)

    def read_config(self, config_path):
        if path.exists(config_path):
            extension_dir = config_path[:-13]
            with open(config_path, mode="r", encoding="utf8") as config_file:
                config = json.load(config_file)
                if "kungfuConfig" in config:
                    if "config" in config["kungfuConfig"]:
                        group = config["kungfuConfig"]["key"]
                        for category in config["kungfuConfig"]["config"]:
                            if category not in kfj.CATEGORIES:
                                raise RuntimeError(f"Unsupported category {category}")
                            if (
                                self.executors["strategy"]["default"]
                                and self.ctx.category == "strategy"
                                and self.ctx.group == "default"
                            ):
                                self.executors["strategy"]["default"].config = config
                            else:
                                self.executors[category][group] = ExtensionLoader(
                                    self.ctx, extension_dir, config
                                )

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
        self["cached"] = lambda mode, low_latency: yjj.cached(
            ctx.runtime_locator, kfj.MODES[ctx.mode], ctx.low_latency
        ).run()
        self["ledger"] = lambda mode, low_latency: wc.Ledger(
            ctx.runtime_locator, kfj.MODES[ctx.mode], ctx.low_latency
        ).run()


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
            "strategy": self.run_strategy,
        }

    def __call__(self, mode, low_latency):
        self.runners[self.ctx.category]()

    def run_broker_vendor(self, vendor_builder):
        ctx = self.ctx
        loader = self.loader
        if loader.extension_dir:
            site.setup(loader.extension_dir)
            sys.path.insert(0, loader.extension_dir)
        module = importlib.import_module(ctx.group)
        vendor = vendor_builder(
            ctx.runtime_locator, ctx.group, ctx.name, ctx.low_latency
        )
        service = getattr(module, ctx.category)(vendor)
        vendor.set_service(service)
        vendor.run()

    def run_market_data(self):
        self.run_broker_vendor(wc.MarketDataVendor)

    def run_trader(self):
        self.run_broker_vendor(wc.TraderVendor)

    def run_strategy(self):
        loader = self.loader
        if loader.extension_dir:
            site.setup(loader.extension_dir)
            sys.path.insert(0, loader.extension_dir)

        ctx = self.ctx
        ctx.location = yjj.location(
            kfj.MODES[ctx.mode],
            lf.enums.category.STRATEGY,
            ctx.group,
            ctx.name,
            ctx.runtime_locator,
        )
        ctx.logger = create_logger(ctx.name, ctx.log_level, ctx.location)
        if loader.config is None:
            ctx.strategy = load_strategy(ctx, ctx.path, loader.config)
        else:
            ctx.strategy = load_strategy(
                ctx, ctx.path, loader.config["kungfuConfig"]["key"]
            )
        ctx.runner = Runner(ctx, kfj.MODES[ctx.mode])
        ctx.runner.add_strategy(ctx.strategy)
        ctx.loop = KungfuEventLoop(ctx, ctx.runner)
        ctx.loop.run_forever()


class RegistryJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        test = isinstance(obj, ExtensionLoader) or isinstance(obj, types.FunctionType)
        return str(obj) if test else obj.__dict__


def load_strategy(ctx, path, key):
    if path.endswith(".py"):
        return Strategy(ctx)  # keep strategy alive for pybind11
    elif key is not None and (path.endswith(".so") or path.endswith(".pyd")):
        ctx.path = os.path.join(os.path.dirname(path), key)
        return Strategy(ctx)
    elif key is not None and path.endswith(key):
        return Strategy(ctx)
    else:
        spec = spec_from_file_location(os.path.basename(path).split(".")[0], path)
        module_cpp = module_from_spec(spec)
        spec.loader.exec_module(module_cpp)
        return module_cpp.strategy()
