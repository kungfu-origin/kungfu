import click
from kungfu.command import kfc, pass_ctx_from_parent, replay_setup
from kungfu.wingchun.watcher import Watcher


@kfc.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.pass_context
def watcher(ctx, low_latency, replay, session_id):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.session_id = session_id
    watcher_instance = Watcher(ctx)
    if replay:
        ctx.category = 'system'
        ctx.group = 'watcher'
        ctx.name = 'watcher'
        replay_setup.setup(ctx, session_id, watcher, watcher_instance)
    watcher_instance.run()
