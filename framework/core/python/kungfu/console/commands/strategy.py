import os
import click
import kungfu

from importlib.util import module_from_spec, spec_from_file_location

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.wingchun.replay import setup
from kungfu.wingchun.strategy import Runner, Strategy
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.practice.coloop import KungfuEventLoop

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing
strategy_command_context = kfc.pass_context(
    "replay", "category", "mode", "location", "logger", "runner", "loop"
)


@kfc.group(cls=PrioritizedCommandGroup)
@click.option("-g", "--group", type=str, default="default", help="group")
@click.option("-n", "--name", type=str, required=True, help="name")
@click.option("-x", "--low_latency", is_flag=True, help="run in low latency mode")
@kfc.pass_context()
def strategy(ctx, group, name, low_latency):
    kfc.pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.category = "strategy"
    mode = lf.enums.mode.REPLAY if ctx.replay else lf.enums.mode.LIVE
    ctx.mode = lf.enums.get_mode_name(mode)
    ctx.location = yjj.location(
        mode, lf.enums.category.STRATEGY, group, name, ctx.runtime_locator
    )
    ctx.logger = create_logger(name, ctx.log_level, ctx.location)
    ctx.runner = Runner(ctx, mode)
    ctx.loop = KungfuEventLoop(ctx, ctx.runner)


@strategy.command(help_priority=1)
@click.option("-p", "--path", type=str, required=True, help="path of strategy py file")
@strategy_command_context
def run(ctx, path):
    ctx.staregy = load_strategy(ctx, path)
    ctx.runner.add_strategy(ctx.strategy)
    ctx.loop.run_forever()


@strategy.command()
@click.option("-p", "--path", type=str, required=True, help="path of strategy py file")
@click.option(
    "-i",
    "--session_id",
    type=int,
    help="replay session id, MUST be specified if replay is set",
)
@strategy_command_context
def replay(ctx, group, name, path, low_latency, replay, session_id):
    ctx.staregy = load_strategy(ctx, path)
    ctx.runner.add_strategy(ctx.strategy)
    ctx.session_id = session_id
    setup(ctx, session_id, strategy, ctx.runner)
    ctx.loop.run_forever()


def load_strategy(ctx, path):
    if path.endswith(".py"):
        return Strategy(ctx)  # keep strategy alive for pybind11
    else:
        spec = spec_from_file_location(os.path.basename(path).split(".")[0], path)
        module_cpp = module_from_spec(spec)
        spec.loader.exec_module(module_cpp)
        return module_cpp.Strategy(ctx.location)
