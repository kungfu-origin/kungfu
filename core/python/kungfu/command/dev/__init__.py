import pyyjj
import json
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.log import create_logger


@kfc.group()
@click.pass_context
def dev(ctx):
    pass_ctx_from_root(ctx)
    ctx.dev_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'dev', 'dev', ctx.locator)
    ctx.io_device = pyyjj.io_device(ctx.dev_location)


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.dev_location = ctx.parent.dev_location
    ctx.io_device = ctx.parent.io_device

