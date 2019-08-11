import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup
from kungfu.wingchun.commander import Commander


@kfc.command(help_priority=2)
@click.pass_context
def commander(ctx):
    pass_ctx_from_parent(ctx)
    commander_instance = Commander(ctx)
    commander_instance.run()
