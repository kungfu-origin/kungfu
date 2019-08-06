import pyyjj
import pywingchun
import click
import os
from kungfu.command import kfc, pass_ctx_from_parent, backtest_setup
from kungfu.wingchun import Runner, replay_setup
from kungfu.wingchun.strategy import Strategy
from kungfu.yijinjing.log import create_logger
from kungfu.wingchun.oms.order import *

@kfc.command()
@click.option('-g', '--group', type=str, default='default', help='group')
@click.option('-n', '--name', type=str, required=True, help='name')
@click.option('-p', '--path', type=str, required=True, help='path of strategy py file')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.option('-b', '--backtest', is_flag=True, help='run in backtest mode')
@click.pass_context
def strategy(ctx, group, name, path, low_latency, replay, session_id, backtest):
    pass_ctx_from_parent(ctx)
    ctx.group = group
    ctx.name = name
    ctx.path = path
    ctx.low_latency = low_latency if not replay else True
    ctx.md_path = None

    assert not(replay and backtest), "Replay mode and BackTest mode cannot be selected together"
    ctx.replay = replay
    ctx.backtest = backtest
    mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.BACKTEST if ctx.backtest else pyyjj.mode.LIVE
    ctx.logger = create_logger(name, ctx.log_level, pyyjj.location(mode, pyyjj.category.STRATEGY, group, name, ctx.locator))

    ctx.strategy = Strategy(ctx)  # keep strategy alive for pybind11
    runner = Runner(ctx, mode)
    runner.add_strategy(ctx.strategy)
    ctx.category = 'strategy'

    if replay:
        ctx.session_id = session_id
        replay_setup.setup(ctx, session_id, strategy, runner)
    if backtest:
        #ctx.md_path = os.path.join(ctx.home, 'md', group, name, 'journal', 'backtest', '00000000.*.journal')
        ctx.category = 'md'
        ctx.mode = pyyjj.get_mode_name(mode)
        ctx.session_id = session_id
        backtest_setup.setup(ctx, session_id, strategy, runner)

    runner.run()

