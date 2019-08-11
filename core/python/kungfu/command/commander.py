import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup
from kungfu.wingchun.commander import Commander


@kfc.command(help_priority=2)
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-n', '--name', type=str, default='commander', help='name')
@click.pass_context
def commander(ctx, name, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency
    ctx.name = name
    commander_instance = Commander(ctx)
    commander_instance.run()
