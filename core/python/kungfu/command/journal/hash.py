import pyyjj
import pywingchun
import click
from kungfu.command.journal import journal, pass_ctx_from_parent
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
from kungfu.practice.apprentice import Apprentice
from kungfu.wingchun.strategy import Strategy

KB = 2 ** 10
MB = 2 ** 20
GB = 2 ** 30

# source_id = 3bd86af2 [live/md/xtp/level1]         from = [2019-06-14 17:17:07.391717989]
# source_id = ec06e15a [live/td/xtp/15040900]       from = [2019-06-14 17:17:48.181709337]

# mode/category/group/id/*.page

# live/system/master/publish/[live/strategy/hft/test].1.page
# live/system/master/ctrl/out.1.page

# live/md/xtp/level1/[0].1.page
# live/md/xtp/level2/out.2.page

# live/td/xtp/15040900/.1.journal
# live/td/xtp/15011218/.1.journal

# live/strategy/hft/test/[live/md/xtp/level1].1.journal
# live/strategy/hft/test/[live/md/xtp/level2].1.journal
# live/strategy/hft/test/[live/td/xtp/15011218].1.journal
# live/strategy/hft/test/[live/td/xtp/15040900].1.journal
# live/strategy/alpha/test.1.journal


@journal.command()
@click.pass_context
def test(ctx):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = False

    # location = pyyjj.location(kfj.MODES[ctx.mode], kfj.CATEGORIES[ctx.category], ctx.group, ctx.name, ctx.locator)
    # md = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.MD, 'xtp', 'xtp', ctx.locator)
    # io_device = pyyjj.io_device_master(location, False)
    # reader = io_device.open_reader(location, md.uid)
    # while reader.data_available():
    #     frame = reader.current_frame()
    #     click.echo('length={}, {}'.format(frame.frame_length, frame.data_as_bytes()))
    #     reader.next()

    # app = Apprentice(ctx)
    # app.run()

    ctx.path = "../examples/strategy/py/strategy_demo.py"
    strategy = Strategy(ctx)
    runner = pywingchun.Runner(ctx.low_latency, ctx.locator, ctx.group, ctx.name)
    runner.add_strategy(strategy)
    ctx.logger.info("replaying")
    replay = pyyjj.replay(runner)
    replay.run()
