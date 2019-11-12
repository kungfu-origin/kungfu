import pyyjj
import pywingchun
import click
from kungfu.command import kfc, pass_ctx_from_parent
from extensions import EXTENSION_REGISTRY_MD
import json

@kfc.command(help_priority=7)
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='data source')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('--time-interval', default="1m", type=str, help="bar time interval")
@click.pass_context
def bar(ctx, source, time_interval, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.mode = pyyjj.mode.LIVE
    args = {"source": source, "time_interval": time_interval}
    instance = pywingchun.BarGenerator(ctx.locator, ctx.mode, low_latency, json.dumps(args))
    instance.run()
