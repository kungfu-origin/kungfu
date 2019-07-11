import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.data.sqlite.data_proxy import make_url, DataProxy
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD
from kungfu.log import create_logger


def run_extension(ctx, registry):
    if ctx.source == 'passive' and registry.has_extension(ctx.source):
        config_str = {
            'user_id': 'test' if not hasattr(ctx, 'account') else ctx.account
        }
        config_int = {}
        config_double = {}
        gateway = registry.get_extension(ctx.source)(ctx.low_latency, ctx.locator, config_str, config_int, config_double)
        gateway.run()
    elif registry.has_extension(ctx.source):
        config = DataProxy(make_url(ctx.locator, ctx.system_config_location, "task")).get_task_config(ctx.name)
        config_str = {}
        config_int = {}
        config_double = {}
        for pname in config:
            if type(config[pname]) == str:
                config_str[pname] = config[pname]
            elif type(config[pname]) == int:
                config_int[pname] = config[pname]
            elif type(config[pname]) == float:
                config_double[pname] = config[pname]
            else:
                ctx.logger.error('unknown config %s, %s', type(config[pname]), config[pname])
        if 'client_id' not in config_int:
            config_int['client_id'] = 1
        config_str['save_file_path'] = '{}/runtime'.format(ctx.home)
        gateway = registry.get_extension(ctx.source)(ctx.low_latency, ctx.locator, config_str, config_int, config_double)
        gateway.run()
    else:
        ctx.logger.error('Unrecognized %s arg %s', registry.ext_type.lower(), ctx.name)


@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def md(ctx, source, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.name = 'md_' + source
    ctx.source = source
    ctx.low_latency = low_latency
    ctx.logger = create_logger(source, ctx.log_level, pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.MD, source, source, ctx.locator))
    run_extension(ctx, EXTENSION_REGISTRY_MD)


@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_TD.names()), help='destination to send order')
@click.option('-a', '--account', type=str, help='account')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def td(ctx, source, account, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.name = 'td_' + source + '_' + account
    ctx.source = source
    ctx.account = account
    ctx.low_latency = low_latency
    ctx.logger = create_logger(source, ctx.log_level, pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, source, account, ctx.locator))
    run_extension(ctx, EXTENSION_REGISTRY_TD)
