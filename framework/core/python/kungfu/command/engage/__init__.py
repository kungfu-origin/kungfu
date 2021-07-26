import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger


@kfc.group()
@click.help_option("-h", "--help")
@click.pass_context
def engage(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = create_logger("engage", ctx.log_level, ctx.console_location)


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
