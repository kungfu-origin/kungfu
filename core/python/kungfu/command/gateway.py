import click
from kungfu.command import kfc
from kungfu.practice.apprentice import Apprentice, EventHandler
from kungfu.data.sqlite import get_task_config
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD, EXTENSION_REGISTRY_LOG


def run_extension(ctx, registry, extension_type):
    if registry.has_extension(ctx.name):
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
        if not 'client_id' in config_int:
            config_int['client_id'] = 1
        config_str['save_file_path'] = '{}/runtime'.format(ctx.home)
        gateway = registry.get_extension(ctx.name)(config_str, config_int, config_double)
        handler = EventHandler(ctx, gateway)
        apprentice = Apprentice(ctx)
        apprentice.add_event_handler(handler)
        apprentice.go()
    else:
        ctx.logger.error('Unrecognized %s arg %s', extension_type, ctx.name)


@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.pass_context
def md(ctx, source):
    ctx.name = 'md_' + source
    run_extension(ctx.parent, EXTENSION_REGISTRY_MD, source, 'md')

@kfc.command()
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_TD.names()), help='destination to send order')
@click.pass_context
def td(ctx, source):
    run_extension(ctx.parent, EXTENSION_REGISTRY_TD, source, 'td')
