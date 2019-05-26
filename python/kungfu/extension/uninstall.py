import os, shutil
import extensions
import kungfu.extension as kfext

@kfext.arg("-n", "--name", dest="name", required=True, help="KungFu extension name")
@kfext.command(help='uninstall extension')
def uninstall(args, logger):
    install_path = os.path.join(extensions.extension_path, args.name)
    if os.path.exists(install_path):
        shutil.rmtree(install_path)
        logger.info('Uninstalled extension %s', args.name)
        print('Uninstalled extension', args.name)
    else:
        print('Extension', args.name, 'not found')
