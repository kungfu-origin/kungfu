import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup

@kfc.group(help_priority=6)
@click.pass_context
def algo(ctx):
    pass_ctx_from_parent(ctx)


