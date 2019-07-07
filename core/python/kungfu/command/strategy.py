import pyyjj
import pywingchun
import click
from kungfu.command import kfc, pass_ctx_from_parent, replay_setup
from kungfu.wingchun.strategy import Strategy
from kungfu.log import create_logger


@kfc.command()
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
    ctx.session_id = session_id
    mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
    ctx.logger = create_logger(name, ctx.log_level, pyyjj.location(mode, pyyjj.category.STRATEGY, group, name, ctx.locator))
    ctx.strategy = Strategy(ctx)  # keep strategy alive for pybind11
    runner = pywingchun.Runner(ctx.locator, ctx.group, ctx.name, mode, ctx.low_latency)
    runner.add_strategy(ctx.strategy)
    if replay:
        ctx.category = 'strategy'
        replay_setup.setup(ctx, session_id, strategy, runner)
    runner.run()
