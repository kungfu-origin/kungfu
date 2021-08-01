import click
import kungfu

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu_extensions import EXTENSION_REGISTRY_MD
from kungfu_extensions import EXTENSION_REGISTRY_TD

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing


@kfc.group(cls=PrioritizedCommandGroup)
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_MD.names()),
    help="data source",
)
@click.option("-x", "--low_latency", is_flag=True, help="run in low latency mode")
@click.pass_context
def vendor(ctx, source, low_latency):
    kfc.pass_ctx_from_parent(ctx)
    config = yjj.location(
        lf.enums.mode.LIVE, lf.enums.category.MD, source, source, ctx.runtime_locator
    ).to(lf.types.Config())
    config = yjj.profile(ctx.runtime_locator).get(config)
    ext = EXTENSION_REGISTRY_MD.get_extension(source)(
        low_latency, ctx.runtime_locator, config.value
    )
    ext.run()


@vendor.command()
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_MD.names()),
    help="data source",
)
@click.pass_context
def md(ctx, source, low_latency):
    kfc.pass_ctx_from_parent(ctx)
    config = yjj.location(
        lf.enums.mode.LIVE, lf.enums.category.MD, source, source, ctx.runtime_locator
    ).to(lf.types.Config())
    config = yjj.profile(ctx.runtime_locator).get(config)
    ext = EXTENSION_REGISTRY_MD.get_extension(source)(
        low_latency, ctx.runtime_locator, config.value
    )
    ext.run()


@vendor.command()
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_TD.names()),
    help="destination to send order",
)
@click.option("-a", "--account", type=str, help="account")
@click.option("-x", "--low_latency", is_flag=True, help="run in low latency mode")
@click.pass_context
def td(ctx, source, account, low_latency):
    kfc.pass_ctx_from_parent(ctx)
    config = yjj.location(
        lf.enums.mode.LIVE, lf.enums.category.TD, source, account, ctx.runtime_locator
    ).to(lf.types.Config())
    config = yjj.profile(ctx.runtime_locator).get(config)
    ext = EXTENSION_REGISTRY_TD.get_extension(source)(
        low_latency, ctx.runtime_locator, account, config.value
    )
    ext.run()
