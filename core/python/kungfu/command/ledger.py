import pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from kungfu.wingchun import replay_setup
from kungfu.wingchun.service.ledger import Ledger


@kfc.command(help_priority=5)
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-r', '--replay', is_flag=True, help='run in replay mode')
@click.option('-i', '--session_id', type=int, help='replay session id, MUST be specified if replay is set')
@click.pass_context
def ledger(ctx, low_latency, replay, session_id):
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency if not replay else True
    ctx.replay = replay
    ctx.category = pyyjj.category.SYSTEM
    ctx.mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
    ctx.group = 'service'
    ctx.name = 'ledger'
    ctx.session_id = session_id
    ledger_instance = Ledger(ctx)
    if replay:
        ctx.category = 'system'
        replay_setup.setup(ctx, session_id, ledger, ledger_instance)
    ledger_instance.run()
