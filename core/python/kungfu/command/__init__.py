import os
import platform
import click
import kungfu.yijinjing.journal as kfj
import pyyjj


@click.group(invoke_without_command=True)
@click.option('-H', '--home', type=str, help='kungfu home folder, defaults to APPDATA/kungfu/app, where APPDATA defaults to %APPDATA% on windows, $XDG_CONFIG_HOME on linux, ~/Library/Application Support on mac')
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('-n', '--name', type=str, help='name for the process, defaults to command if not set')
@click.pass_context
def kfc(ctx, home, log_level, name):
    if not home:
        osname = platform.system()
        user_home = os.path.expanduser('~')
        if osname == 'Linux':
            xdg_config_home = os.getenv('XDG_CONFIG_HOME')
            home = xdg_config_home if xdg_config_home else os.path.join(user_home, '.config')
        if osname == 'Darwin':
            home = os.path.join(user_home, 'Library', 'Application Support')
        if osname == 'Windows':
            home = os.getenv('APPDATA')
        home = os.path.join(home, 'kungfu', 'app')

    os.environ['KF_HOME'] = ctx.home = home
    os.environ['KF_LOG_LEVEL'] = ctx.log_level = log_level

    # have to keep locator alive from python side
    # https://github.com/pybind/pybind11/issues/1546
    ctx.locator = kfj.Locator(home)
    ctx.system_config_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator)

    if ctx.invoked_subcommand is None:
        click.echo(kfc.get_help(ctx))
    else:
        ctx.name = name if name else ctx.invoked_subcommand
    pass


def pass_ctx_from_parent(ctx):
    ctx.home = ctx.parent.home
    ctx.log_level = ctx.parent.log_level
    ctx.locator = ctx.parent.locator
    ctx.system_config_location = ctx.parent.system_config_location
    ctx.name = ctx.parent.name


def execute():
    kfc(auto_envvar_prefix='KF')
