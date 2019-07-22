from . import ${PROJECT_NAME} as kfext_bar
import click
from kungfu.command import kfc, pass_ctx_from_parent
import pyyjj

@kfc.command()
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.option('-s', '--source',type=str, help='md source')
@click.option('-f', '--frequency',type=int, help='bar frequency')
@click.pass_context
def bar_handle(ctx, source, frequency, low_latency):
    print(source, frequency, low_latency)
    pass_ctx_from_parent(ctx)
    ctx.low_latency = low_latency
    location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.MD, source, str(frequency)+"_min", ctx.locator)
    server = kfext_bar.bar(location, source, frequency, low_latency)
    server.run()