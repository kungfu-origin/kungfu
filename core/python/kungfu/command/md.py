from pykungfu import longfist
from pykungfu import yijinjing as yjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu_extensions import EXTENSION_REGISTRY_MD


@kfc.command(help_priority=3)
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def md(ctx, source, low_latency):
    pass_ctx_from_parent(ctx)
    config = yjj.location(yjj.mode.LIVE, yjj.category.MD, source, source, ctx.runtime_locator).to(longfist.types.Config())
    config = yjj.profile(ctx.runtime_locator).get(config)
    ext = EXTENSION_REGISTRY_MD.get_extension(source)(low_latency, ctx.runtime_locator, config.value)
    ext.run()
