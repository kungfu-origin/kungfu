import click
import shutil
import kungfu.yijinjing.journal as kfj
from pykungfu import yijinjing as yjj
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger


@kfc.group(help_priority=5)
@click.option(
    "-m", "--mode", default="*", type=click.Choice(kfj.MODES.keys()), help="mode"
)
@click.option(
    "-c",
    "--category",
    default="*",
    type=click.Choice(kfj.CATEGORIES.keys()),
    help="category",
)
@click.option("-g", "--group", type=str, default="*", help="group")
@click.option("-n", "--name", type=str, default="*", help="name")
@click.help_option("-h", "--help")
@click.pass_context
def journal(ctx, mode, category, group, name):
    pass_ctx_from_root(ctx)
    ctx.low_latency = False
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.location = yjj.location(
        kfj.MODES[mode], kfj.CATEGORIES[category], group, name, ctx.runtime_locator
    )
    ctx.logger = create_logger("journal", ctx.log_level, ctx.console_location)

    (ctx.console_width, ctx.console_height) = shutil.get_terminal_size((0, 0))
    yjj.setup_log(ctx.console_location, "journal")


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
    ctx.low_latency = ctx.parent.low_latency
    ctx.mode = ctx.parent.mode
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.runtime_dir = ctx.parent.runtime_dir
    ctx.archive_dir = ctx.parent.archive_dir
    ctx.dataset_dir = ctx.parent.dataset_dir
    ctx.inbox_dir = ctx.parent.inbox_dir
    ctx.runtime_locator = ctx.parent.runtime_locator
    ctx.config_location = ctx.parent.config_location
    ctx.console_location = ctx.parent.console_location
    ctx.index_location = ctx.parent.index_location
    ctx.location = ctx.parent.location
    ctx.console_width = ctx.parent.console_width
    ctx.console_height = ctx.parent.console_height
