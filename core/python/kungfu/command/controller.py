import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup
from kungfu.wingchun.controller import Controller


@kfc.command()
@click.pass_context
def controller(ctx, low_latency, replay, session_id):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.session_id = session_id
    controller_instance = Controller(ctx)
    controller_instance.run()