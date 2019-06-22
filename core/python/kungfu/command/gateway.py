import click
from kungfu.command import kfc
from kungfu.data.sqlite import get_task_config
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD


def run_extension(ctx, registry):
    if registry.has_extension(ctx.source):
        config = get_task_config(ctx.name)
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
        gateway = registry.get_extension(ctx.source)(config_str, config_int, config_double)
        gateway.run()
    else:
        ctx.logger.error('Unrecognized %s arg %s', registry.ext_type.lower(), ctx.name)


@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def md(ctx, source, low_latency):
    ctx.parent.name = 'md_' + source
    ctx.parent.source = source
    ctx.parent.low_latency = low_latency
    run_extension(ctx.parent, EXTENSION_REGISTRY_MD)


@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_TD.names()), help='destination to send order')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def td(ctx, source, low_latency):
    ctx.parent.source = source
    ctx.parent.low_latency = low_latency
    run_extension(ctx.parent, EXTENSION_REGISTRY_TD)
