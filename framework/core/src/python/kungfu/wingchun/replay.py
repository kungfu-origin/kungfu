import click
import kungfu
import sys

from tabulate import tabulate
from kungfu.yijinjing import time as kft
from kungfu.yijinjing import journal as kfj

lf = kungfu.__binding__.longfist
yjj = kungfu.__binding__.yijinjing


def setup(ctx, session_id, cmd, instance):
    home = instance.home
    ctx.app_location = yjj.location(
        lf.enums.mode.LIVE, home.category, home.group, home.name, ctx.runtime_locator
    )
    ctx.mode = "live"  # to get live data
    if not session_id:
        all_sessions = kfj.find_sessions(ctx)
        all_sessions["begin_time"] = all_sessions["begin_time"].apply(
            lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT)
        )
        all_sessions["end_time"] = all_sessions["end_time"].apply(
            lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT)
        )
        all_sessions["duration"] = all_sessions["duration"].apply(
            lambda t: kft.strftime(t - kft.DURATION_TZ_ADJUST, kft.DURATION_FORMAT)
        )
        click.echo(cmd.get_help(ctx))
        click.echo("please select sessions:")
        click.echo(
            tabulate(
                all_sessions.values, headers=all_sessions.columns, tablefmt="simple"
            )
        )
        sys.exit()
    else:
        session = kfj.find_session(ctx, session_id)
        instance.set_begin_time(session["begin_time"])
        instance.set_end_time(session["end_time"])
