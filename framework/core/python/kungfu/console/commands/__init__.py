import click
import kungfu
import platform
import os
import typing

from click.decorators import F as CLI
from click.globals import get_current_context
from functools import update_wrapper


def pick(**kwargs):
    from . import __registry__ as commands

    kfc(obj=commands, **kwargs)


class PrioritizedCommandGroup(click.Group):
    DEFAULT_PRIORITY = 100

    def __init__(self, *args, **kwargs):
        self.help_priorities = {}
        self.list_commands = self.list_commands_for_help
        super(PrioritizedCommandGroup, self).__init__(*args, **kwargs)

    def get_help(self, ctx):
        return super(PrioritizedCommandGroup, self).get_help(ctx)

    def list_commands_for_help(self, ctx):
        """reorder the list of commands when listing the help"""
        commands = super(PrioritizedCommandGroup, self).list_commands(ctx)
        commands = filter(lambda command: self.help_priorities[command] > 0, commands)
        return (
            c[1]
            for c in sorted(
                (self.help_priorities.get(command, self.DEFAULT_PRIORITY), command)
                for command in commands
            )
        )

    def group(self, *args, **kwargs):
        """Behaves the same as `click.Group.command()` except capture
        a priority for listing command names in help.
        """
        help_priority = kwargs.pop("help_priority", self.DEFAULT_PRIORITY)
        help_priorities = self.help_priorities

        def decorator(f):
            group = super(PrioritizedCommandGroup, self).group(*args, **kwargs)(f)
            help_priorities[group.name] = help_priority
            return group

        return decorator

    def command(self, *args, **kwargs):
        """Behaves the same as `click.Group.command()` except capture
        a priority for listing command names in help.
        """
        help_priority = kwargs.pop("help_priority", self.DEFAULT_PRIORITY)
        help_priorities = self.help_priorities

        def decorator(f):
            cmd = super(PrioritizedCommandGroup, self).command(*args, **kwargs)(f)
            help_priorities[cmd.name] = help_priority
            return cmd

        return decorator

    @staticmethod
    def pass_context(*keys):
        def copy_from_parent(f: CLI) -> CLI:
            def new_func(*args, **kwargs):
                ctx = get_current_context()
                for key in [
                    "name",
                    "home",
                    "log_level",
                    "runtime_dir",
                    "archive_dir",
                    "dataset_dir",
                    "inbox_dir",
                    "runtime_locator",
                    "config_location",
                    "console_location",
                    "index_location",
                ] + list(keys):
                    ctx.__dict__[key] = ctx.parent.__dict__[key]
                return f(ctx, *args, **kwargs)

            return update_wrapper(typing.cast(CLI, new_func), f)

        return copy_from_parent


@click.group("kfc", invoke_without_command=True, cls=PrioritizedCommandGroup)
@click.option(
    "-H",
    "--home",
    type=str,
    help="kungfu home folder, defaults to APPDATA/kungfu/home/runtime, "
    "where APPDATA defaults to %APPDATA% on windows, "
    "~/.config on linux, ~/Library/Application Support on mac",
)
@click.option(
    "-l",
    "--log_level",
    type=click.Choice(["trace", "debug", "info", "warning", "error", "critical"]),
    default="warning",
    help="logging level",
)
@click.option(
    "-n",
    "--name",
    type=str,
    help="name for the process, defaults to command if not set",
)
@click.option(
    "-c",
    "--code",
    type=str,
    help="python code passed in as string",
)
@click.help_option("-h", "--help")
@click.version_option(kungfu.__version__, "--version", message=kungfu.__version__)
@click.pass_context
def kfc(ctx, home, log_level, name, code):
    if not home:
        osname = platform.system()
        user_home = os.path.expanduser("~")
        if osname == "Linux":
            xdg_config_home = os.getenv("XDG_CONFIG_HOME")
            home = (
                xdg_config_home
                if xdg_config_home
                else os.path.join(user_home, ".config")
            )
        if osname == "Darwin":
            home = os.path.join(user_home, "Library", "Application Support")
        if osname == "Windows":
            home = os.getenv("APPDATA")
        home = os.path.join(home, "kungfu", "home")

    os.environ["KF_HOME"] = ctx.home = home
    os.environ["KF_LOG_LEVEL"] = ctx.log_level = log_level

    def ensure_dir(ctx, name):
        target = os.path.join(ctx.home, name)
        if not os.path.exists(target):
            os.makedirs(target)
        return target

    ctx.runtime_dir = ensure_dir(ctx, "runtime")
    ctx.archive_dir = ensure_dir(ctx, "archive")
    ctx.dataset_dir = ensure_dir(ctx, "dataset")
    ctx.inbox_dir = ensure_dir(ctx, "inbox")

    from kungfu.yijinjing.locator import Locator

    lf = kungfu.__bindings__.longfist
    yjj = kungfu.__bindings__.yijinjing

    # have to keep locator alive from python side
    # https://github.com/pybind/pybind11/issues/1546
    ctx.runtime_locator = Locator(ctx.runtime_dir)
    ctx.config_location = yjj.location(
        lf.enums.mode.LIVE,
        lf.enums.category.SYSTEM,
        "etc",
        "kungfu",
        ctx.runtime_locator,
    )
    ctx.console_location = yjj.location(
        lf.enums.mode.LIVE,
        lf.enums.category.SYSTEM,
        "service",
        "console",
        ctx.runtime_locator,
    )
    ctx.index_location = yjj.location(
        lf.enums.mode.LIVE,
        lf.enums.category.SYSTEM,
        "journal",
        "index",
        ctx.runtime_locator,
    )

    ctx.name = name if name else ctx.invoked_subcommand

    if code is not None:
        exec(code)

    if ctx.invoked_subcommand is None and code is None:
        click.echo(kfc.get_help(ctx))

    pass
