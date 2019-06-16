import pyyjj
import click
from kungfu.command.journal import journal
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj

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
    click.echo(2 ** 16)
    click.echo(2 ** 32)
    click.echo(hex(pyyjj.hash_str_32('helloworld'))[2:])
    click.echo(hex(pyyjj.hash_str_32('test'))[2:])
    click.echo(kft.strftime(pyyjj.now_in_nano()))
    loc = pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['td'], "xtp", "15040900", ctx.parent.locator)
    click.echo(loc.uname)
    click.echo(hex(loc.uid))
    click.echo(hex(pyyjj.hash_str_32(loc.uname)))
    click.echo(pyyjj.get_page_path(loc, 0, 1))
    click.echo(pyyjj.get_page_path(loc, loc.uid, 1))
    click.echo(ctx.parent.locator.list_page_id(loc, pyyjj.hash_str_32('helloworld')))
    io = pyyjj.io_device_client(loc, False)
    click.echo('done')
