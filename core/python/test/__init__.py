import os
import platform
import click
import extensions
import kungfu.yijinjing.journal as kfj


@click.group(invoke_without_command=True)
@click.option('-H', '--home', type=str, help="kungfu home folder, defaults to APPDATA/kungfu/app, where APPDATA defaults to %APPDATA% on windows, "
                                             "~/.config or $XDG_CONFIG_HOME (if set) on linux, ~/Library/Application Support on mac")
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.pass_context
def test(ctx, home, log_level):
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
    ctx.locator = kfj.Locator(home)

    if ctx.invoked_subcommand is None:
        click.echo(test.get_help(ctx))
    pass


def pass_ctx_from_parent(ctx):
    ctx.log_level = ctx.parent.log_level
    ctx.locator = ctx.parent.locator
