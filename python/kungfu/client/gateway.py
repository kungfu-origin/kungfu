import nnpy, pyyjj
from kungfu.data.sqlite import get_task_config
from kungfu.command import command, arg
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD

def run_extension(registry, args, logger, extension_name, extension_type):
    if registry.has_extension(extension_name):
        config = get_task_config(args.name)
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
                logger.error('unknown config %s, %s', type(config[pname]), config[pname])
        if not 'client_id' in config_int:
            config_int['client_id'] = 1
        config_str['save_file_path'] = '{}/runtime'.format(args.home)
        plugin = registry.get_extension(extension_name)(config_str, config_int, config_double)
        plugin.init()
        plugin.start()
    else:
        logger.error('Unrecognized %s arg %s', extension_type, extension_name)

@arg('-s', '--source', dest='source', type=str, required=True, choices=EXTENSION_REGISTRY_MD.names(), help='data source')
@command(help='market data gateway')
def md(args, logger):
    args.name = 'md_' + args.source
    run_extension(EXTENSION_REGISTRY_MD, args, logger, args.source, 'md')

@arg('-d', '--dest', dest='dest', type=str, required=True, choices=EXTENSION_REGISTRY_TD.names(), help='destination to send order')
@command(help='trade gateway')
def td(args, logger):
    run_extension(EXTENSION_REGISTRY_TD, args, logger, args.dest, 'td')
