import os
import pyyjj
import click
from importlib import util
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import Runner, replay_setup
from kungfu.wingchun.strategy import Strategy
from kungfu.yijinjing.log import create_logger


@kfc.command(help_priority=4)
@click.option('-g', '--group', type=str, default='default', help='group')
@click.option('-n', '--name', type=str, required=True, help='name')
@click.option('-p', '--path', type=str, required=True, help='path of strategy py file')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.pass_context
def strategy(ctx, group, name, path, low_latency, replay, session_id):
    pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.path = path
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.category = 'strategy'
    mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
    ctx.mode = pyyjj.get_mode_name(mode)
    ctx.location = pyyjj.location(mode, pyyjj.category.STRATEGY, group, name, ctx.locator)
    ctx.logger = create_logger(name, ctx.log_level, ctx.location)

    if path.endswith('.py'):
        ctx.strategy = Strategy(ctx)  # keep strategy alive for pybind11
    else:
        spec = util.spec_from_file_location(os.path.basename(path).split('.')[0], path)
        cpp = util.module_from_spec(spec)
        spec.loader.exec_module(cpp)
        ctx.strategy = cpp.Strategy(ctx.location)

    runner = Runner(ctx, mode)
    runner.add_strategy(ctx.strategy)

    if replay:
        ctx.session_id = session_id
        replay_setup.setup(ctx, session_id, strategy, runner)

    runner.run()

