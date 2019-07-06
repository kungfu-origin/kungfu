import os
import click
import kungfu.yijinjing.journal as kfj


@click.group(invoke_without_command=True)
@click.option('-H', '--home', type=str, required=True, help='kungfu home folder')
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('-n', '--name', type=str, help='name for the process, defaults to command if not set')
@click.pass_context
def kfc(ctx, home, log_level, name):
    os.environ['KF_HOME'] = ctx.home = home
    os.environ['KF_LOG_LEVEL'] = ctx.log_level = log_level

    # have to keep locator alive from python side
    # https://github.com/pybind/pybind11/issues/1546
    ctx.locator = kfj.Locator(home)
    if ctx.invoked_subcommand is None:
        click.echo(kfc.get_help(ctx))
    else:
        ctx.name = name if name else ctx.invoked_subcommand
    pass


def pass_ctx_from_parent(ctx):
    ctx.home = ctx.parent.home
    ctx.log_level = ctx.parent.log_level
    ctx.locator = ctx.parent.locator
    ctx.name = ctx.parent.name


def execute():
    kfc(auto_envvar_prefix='KF')
