import os
import sys
import platform
import json
import click
import kungfu.yijinjing.journal as kfj
import pyyjj
from kungfu import __version__

DEFAULT_CMD_PRIORITY = 100

class SpecialHelpOrder(click.Group):

    def __init__(self, *args, **kwargs):
        self.help_priorities = {}
        super(SpecialHelpOrder, self).__init__(*args, **kwargs)

    def get_help(self, ctx):
        self.list_commands = self.list_commands_for_help
        return super(SpecialHelpOrder, self).get_help(ctx)

    def list_commands_for_help(self, ctx):
        """reorder the list of commands when listing the help"""
        commands = super(SpecialHelpOrder, self).list_commands(ctx)
        return (c[1] for c in sorted(
            (self.help_priorities.get(command, DEFAULT_CMD_PRIORITY), command)
            for command in commands))

    def group(self, *args, **kwargs):
        """Behaves the same as `click.Group.command()` except capture
        a priority for listing command names in help.
        """
        help_priority = kwargs.pop('help_priority', DEFAULT_CMD_PRIORITY)
        help_priorities = self.help_priorities

        def decorator(f):
            group = super(SpecialHelpOrder, self).group(*args, **kwargs)(f)
            help_priorities[group.name] = help_priority
            return group

        return decorator

    def command(self, *args, **kwargs):
        """Behaves the same as `click.Group.command()` except capture
        a priority for listing command names in help.
        """
        help_priority = kwargs.pop('help_priority', DEFAULT_CMD_PRIORITY)
        help_priorities = self.help_priorities

        def decorator(f):
            cmd = super(SpecialHelpOrder, self).command(*args, **kwargs)(f)
            help_priorities[cmd.name] = help_priority
            return cmd

        return decorator


@click.group(invoke_without_command=True, cls=SpecialHelpOrder)
@click.option('-H', '--home', type=str, help="kungfu home folder, defaults to APPDATA/kungfu/app, where APPDATA defaults to %APPDATA% on windows, "
                                             "~/.config or $XDG_CONFIG_HOME (if set) on linux, ~/Library/Application Support on mac")
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('-n', '--name', type=str, help='name for the process, defaults to command if not set')
@click.version_option(__version__, '--version', '-v', message = 'version {}'.format(__version__))
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

    settings_path = os.path.join(home, 'settings.json')
    if not os.path.exists(settings_path):
        default_settings_file = open(settings_path, 'w+')
        default_settings_file.write('{}')
        default_settings_file.close()
    with open(settings_path) as settings_file:
        ctx.settings = json.load(settings_file)
        settings_file.close()

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
    ctx.settings = ctx.parent.settings
    ctx.locator = ctx.parent.locator
    ctx.system_config_location = ctx.parent.system_config_location
    ctx.name = ctx.parent.name

def execute():
    kfc(auto_envvar_prefix='KF')

