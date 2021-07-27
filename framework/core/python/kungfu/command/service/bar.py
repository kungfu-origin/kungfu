import click
import json
import kungfu

from kungfu.command import kfc, pass_ctx_from_parent
from kungfu_extensions import EXTENSION_REGISTRY_MD

lf = kungfu.__bindings__.longfist
wc = kungfu.__bindings__.wingchun
yjj = kungfu.__bindings__.yijinjing


@kfc.command(help_priority=7)
@click.option(
    "-s",
    "--source",
    required=True,
    type=click.Choice(EXTENSION_REGISTRY_MD.names()),
    help="data source",
)
@click.option("-x", "--low_latency", is_flag=True, help="run in low latency mode")
@click.option(
    "-t",
    "--time-interval",
    default="1m",
    type=str,
    help="bar time interval, s/m/h/d, s=Second m=Minute h=Hour d=Day",
)
@click.pass_context
def bar(ctx, source, time_interval, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.mode = lf.enums.mode.LIVE
    args = {"source": source, "time_interval": time_interval}
    instance = wc.BarGenerator(
        ctx.runtime_locator, ctx.mode, low_latency, json.dumps(args)
    )
    instance.run()
