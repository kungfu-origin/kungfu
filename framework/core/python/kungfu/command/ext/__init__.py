from pykungfu import yijinjing as yjj
import os
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger


@kfc.group()
@click.help_option("-h", "--help")
@click.pass_context
def ext(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = create_logger("ext", ctx.log_level, ctx.console_location)
    if not os.getenv("KF_NO_EXT"):
        pass
    else:
        print("Extension disabled by KF_NO_EXT")
        ctx.logger.warning("Trying to manage extension while disallowed by KF_NO_EXT")


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger
