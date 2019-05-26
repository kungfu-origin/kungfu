import extensions
import kungfu.extension as kfext

@kfext.command(help='list extensions')
def list(args, logger):
    print("Installed extensions:")
    for ext_name in extensions.EXTENSIONS.keys():
        ext_funcs = []
        if extensions.EXTENSION_REGISTRY_MD.has_extension(ext_name):
            ext_funcs.append('md')
        if extensions.EXTENSION_REGISTRY_TD.has_extension(ext_name):
            ext_funcs.append('td')
        print(ext_name, ext_funcs)