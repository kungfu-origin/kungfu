import click
import json
import kungfu

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing.practice.executor import ExecutorRegistry
from kungfu.yijinjing.practice.master import Master
from kungfu.wingchun.replay import setup

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing
service_command_context = kfc.pass_context("low_latency")


@kfc.command(help_priority=1)
@click.option(
    "-m", "--mode", default="live", type=click.Choice(kfj.MODES.keys()), help="mode"
)
@click.option(
    "-c",
    "--category",
    type=click.Choice(kfj.CATEGORIES.keys()),
    help="category",
)
@click.option("-g", "--group", type=str, help="group")
@click.option("-n", "--name", type=str, help="name")
@click.option("-x", "--low-latency", is_flag=True, help="run in low latency mode")
@click.argument("reference", type=str, required=False)
@kfc.pass_context()
def run(ctx, mode, category, group, name, low_latency, reference):
    ctx.mode = mode
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.low_latency = low_latency
    ctx.path = reference

    registry = ExecutorRegistry(ctx)

    cheatsheet = {
        "master": registry["system"]["master"]["master"],
        "ledger": registry["system"]["service"]["ledger"],
    }

    if not category and not reference:
        click.echo(run.get_help(ctx))
    elif reference in cheatsheet:
        cheatsheet[reference](mode, low_latency)
    else:
        registry[category][group][name](mode, low_latency)


@kfc.command(cls=PrioritizedCommandGroup, help_priority=-1)
@kfc.pass_context()
def service(ctx):
    pass


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
