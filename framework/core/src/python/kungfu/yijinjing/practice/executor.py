import importlib
import json
import os
import sys
import types
import kungfu

from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.practice.master import Master

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
        self["ledger"] = lambda mode, low_latency: wc.Ledger(ctx).run()


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

    def __call__(self, mode, low_latency):
        ctx = self.ctx
        loader = self.loader
        ctx.location = yjj.location(
            kfj.MODES[mode], kfj.CATEGORIES[ctx.category], ctx.group, ctx.name, ctx.runtime_locator
        )
        sys.path.insert(0, loader.extension_dir)
        module = importlib.import_module(ctx.group)
        executor = getattr(module, ctx.category)
        executor(ctx)


class RegistryJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        test = isinstance(obj, ExtensionLoader) or isinstance(obj, types.FunctionType)
        return str(obj) if test else obj.__dict__
