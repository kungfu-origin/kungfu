import click
import json
import kungfu

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing.practice.master import Master
from kungfu.wingchun.replay import setup
from kungfu_extensions import EXTENSION_REGISTRY_MD

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing
service_command_context = kfc.pass_context("low_latency")


@kfc.group(cls=PrioritizedCommandGroup)
@click.option("-x", "--low-latency", is_flag=True, help="run in low latency mode")
@kfc.pass_context()
def service(ctx, low_latency):
    ctx.low_latency = low_latency


@service.command(help_priority=1)
@service_command_context
def master(ctx):
    Master(ctx).run()


@service.command()
@click.option("-r", "--replay", is_flag=True, help="run in replay mode")
@click.option(
    "-i",
    "--session_id",
    type=int,
    help="replay session id, MUST be specified if replay is set",
)
@service_command_context
def ledger(ctx, replay, session_id):
    ctx.low_latency = ctx.low_latency if not replay else True
    ctx.replay = replay
    ctx.category = lf.enums.category.SYSTEM
    ctx.mode = lf.enums.mode.REPLAY if ctx.replay else lf.enums.mode.LIVE
    ctx.group = "service"
    ctx.name = "ledger"
    ctx.session_id = session_id
    ledger_instance = wc.Ledger(ctx.runtime_locator, ctx.mode, ctx.low_latency)
    if replay:
        ctx.category = "system"
        setup(ctx, session_id, ledger, ledger_instance)
    ledger_instance.run()


@service.command()
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_MD.names()),
    help="data source",
)
@click.option(
    "-t",
    "--time-interval",
    default="1m",
    type=str,
    help="bar time interval, s/m/h/d, s=Second m=Minute h=Hour d=Day",
)
@service_command_context
def bar(ctx, source, time_interval):
    ctx.mode = lf.enums.mode.LIVE
    args = {"source": source, "time_interval": time_interval}
    instance = wc.BarGenerator(
        ctx.runtime_locator, ctx.mode, ctx.low_latency, json.dumps(args)
    )
    instance.run()