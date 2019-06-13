import os, shutil
import extensions
import click
from kungfu.command.extension import extension as kfext


@kfext.command(help='uninstall extension')
@click.option("-n", "--name", required=True, help="KungFu extension name")
@click.pass_context
def uninstall(ctx, name):
    install_path = os.path.join(extensions.extension_path, name)
    if os.path.exists(install_path):
        shutil.rmtree(install_path)
        ctx.parent.logger.info('Uninstalled extension %s', name)
        click.echo('Uninstalled extension', name)
    else:
        click.echo('Extension', name, 'not found')
