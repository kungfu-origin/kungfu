import click
from kungfu.command.data import data, pass_ctx_from_parent
from kungfu_extensions import EXTENSION_REGISTRY_DATA
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj


@data.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_DATA.names()), help='data source')
@click.option('-n', '--name', required=True, type=str, help='dataset name')
@click.pass_context
def get(ctx, source, name):
    pass_ctx_from_parent(ctx)
    # location = yjj.location(lf.enums.mode.DATA, lf.enums.category.MD, source, source, ctx.runtime_locator)
    # ctx.config = yjj.profile(ctx.runtime_locator).get(location.to(lf.types.Config()))
    ctx.dataset_name = name
    ext = EXTENSION_REGISTRY_DATA.get_extension(source)(ctx)
    ext.run()
