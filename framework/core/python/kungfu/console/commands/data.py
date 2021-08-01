import click
import kungfu

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing.log import create_logger
from kungfu_extensions import EXTENSION_REGISTRY_DATA

yjj = kungfu.__bindings__.yijinjing


@kfc.group(cls=PrioritizedCommandGroup)
@click.help_option("-h", "--help")
@click.pass_context
def data(ctx):
    kfc.pass_ctx_from_parent(ctx)
    ctx.low_latency = False
    ctx.logger = create_logger("data", ctx.log_level, ctx.console_location)
    yjj.setup_log(ctx.console_location, "data")


def pass_ctx_from_parent(ctx):
    kfc.pass_ctx_from_parent(ctx)
    ctx.logger = ctx.parent.logger
    ctx.low_latency = ctx.parent.low_latency
    ctx.runtime_dir = ctx.parent.runtime_dir
    ctx.archive_dir = ctx.parent.archive_dir
    ctx.dataset_dir = ctx.parent.dataset_dir
    ctx.inbox_dir = ctx.parent.inbox_dir
    ctx.runtime_locator = ctx.parent.runtime_locator
    ctx.config_location = ctx.parent.config_location
    ctx.console_location = ctx.parent.console_location
    ctx.index_location = ctx.parent.index_location


@data.command()
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_DATA.names()),
    help="data source",
)
@click.option("-n", "--name", required=True, type=str, help="dataset name")
@click.pass_context
def get(ctx, source, name):
    pass_ctx_from_parent(ctx)
    # location = yjj.location(lf.enums.mode.DATA, lf.enums.category.MD, source, source, ctx.runtime_locator)
    # ctx.config = yjj.profile(ctx.runtime_locator).get(location.to(lf.types.Config()))
    ctx.dataset_name = name
    ext = EXTENSION_REGISTRY_DATA.get_extension(source)(ctx)
    ext.run()
