import os
import platform
import click
from kungfu import __version__
from kungfu.yijinjing.locator import Locator
from pykungfu import yijinjing as yjj

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


def recursive_help(cmd, parent=None):
    ctx = click.core.Context(cmd, info_name=cmd.name, parent=parent)
    print(cmd.get_help(ctx))
    print()
    commands = getattr(cmd, 'commands', {})
    for sub in commands.values():
        recursive_help(sub, ctx)


def ensure_dir(ctx, name):
    target = os.path.join(ctx.home, name)
    if not os.path.exists(target):
        os.makedirs(target)
    return target


@click.group(invoke_without_command=True, cls=SpecialHelpOrder)
@click.option('-H', '--home', type=str, help="kungfu home folder, defaults to APPDATA/kungfu/app, where APPDATA defaults to %APPDATA% on windows, "
                                             "~/.config or $XDG_CONFIG_HOME (if set) on linux, ~/Library/Application Support on mac")
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('-n', '--name', type=str, help='name for the process, defaults to command if not set')
@click.help_option('-h', '--help')
@click.version_option(__version__, '-v', '--version', message='{}'.format(__version__))
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

    ctx.runtime_dir = ensure_dir(ctx, 'runtime')
    ctx.archive_dir = ensure_dir(ctx, 'archive')

    # have to keep locator alive from python side
    # https://github.com/pybind/pybind11/issues/1546
    ctx.runtime_locator = Locator(ctx.runtime_dir)
    ctx.assemble_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'journal', 'assemble', ctx.runtime_locator)
    ctx.index_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'journal', 'index', ctx.runtime_locator)
    ctx.config_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'etc', 'kungfu', ctx.runtime_locator)

    if ctx.invoked_subcommand is None:
        click.echo(kfc.get_help(ctx))
    else:
        ctx.name = name if name else ctx.invoked_subcommand
    pass


def pass_ctx_from_parent(ctx):
    ctx.home = ctx.parent.home
    ctx.log_level = ctx.parent.log_level
    ctx.runtime_dir = ctx.parent.runtime_dir
    ctx.archive_dir = ctx.parent.archive_dir
    ctx.runtime_locator = ctx.parent.runtime_locator
    ctx.assemble_location = ctx.parent.assemble_location
    ctx.index_location = ctx.parent.index_location
    ctx.config_location = ctx.parent.config_location
    ctx.name = ctx.parent.name


def execute():
    kfc(auto_envvar_prefix='KF')
