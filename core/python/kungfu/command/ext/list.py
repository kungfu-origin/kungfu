import extensions
import click
from kungfu.command.ext import ext as kfext, pass_ctx_from_parent


@kfext.command(help='list extensions')
@click.pass_context
def list(ctx):
    pass_ctx_from_parent(ctx)
    click.echo("Installed extensions:")
    for ext_name in extensions.EXTENSIONS.keys():
        ext_funcs = []
        if extensions.EXTENSION_REGISTRY_MD.has_extension(ext_name):
            ext_funcs.append('md')
        if extensions.EXTENSION_REGISTRY_TD.has_extension(ext_name):
            ext_funcs.append('td')
        click.echo('%s %s' % (ext_name, ext_funcs))
