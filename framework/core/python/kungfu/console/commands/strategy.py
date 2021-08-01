import os
import click
import kungfu
import importlib

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.wingchun.replay import setup
from kungfu.wingchun.strategy import Runner, Strategy
from kungfu.yijinjing.log import create_logger
from kungfu.yijinjing.practice.coloop import KungfuEventLoop

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing


@kfc.command(cls=PrioritizedCommandGroup)
@click.option("-g", "--group", type=str, default="default", help="group")
@click.option("-n", "--name", type=str, required=True, help="name")
@click.option("-p", "--path", type=str, required=True, help="path of strategy py file")
@click.option("-x", "--low_latency", is_flag=True, help="run in low latency mode")
@click.option("-r", "--replay", is_flag=True, help="run in replay mode")
@click.option(
    "-i",
    "--session_id",
    type=int,
    help="replay session id, MUST be specified if replay is set",
)
@click.pass_context
def strategy(ctx, group, name, path, low_latency, replay, session_id):
    kfc.pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.path = path
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.category = "strategy"
    mode = lf.enums.mode.REPLAY if ctx.replay else lf.enums.mode.LIVE
    ctx.mode = lf.enums.get_mode_name(mode)
    ctx.location = yjj.location(
        mode, lf.enums.category.STRATEGY, group, name, ctx.runtime_locator
    )
    ctx.logger = create_logger(name, ctx.log_level, ctx.location)

    runner = Runner(ctx, mode)

    if path.endswith(".py"):
        ctx.strategy = Strategy(ctx)  # keep strategy alive for pybind11
    else:
        spec = importlib.util.spec_from_file_location(
            os.path.basename(path).split(".")[0], path
        )
        cpp = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cpp)
        ctx.strategy = cpp.Strategy(ctx.location)

    runner.add_strategy(ctx.strategy)

    if replay:
        ctx.session_id = session_id
        setup(ctx, session_id, strategy, runner)

    ctx.loop = KungfuEventLoop(ctx, runner)
    ctx.loop.run_forever()
