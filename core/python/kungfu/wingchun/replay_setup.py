import sys
import click
from kungfu.yijinjing import time as kft
from kungfu.yijinjing import journal as kfj
from pykungfu import yijinjing as yjj

from tabulate import tabulate


def setup(ctx, session_id, cmd, instance):
    home = instance.home
    ctx.app_location = yjj.location(yjj.mode.LIVE, home.category, home.group, home.name, ctx.locator)
    ctx.mode = 'live'  # to get live data
    ctx.journal_util_location = yjj.location(yjj.mode.LIVE, yjj.category.SYSTEM, 'util', 'journal', ctx.locator)
    if not session_id:
        all_sessions = kfj.find_sessions(ctx)
        all_sessions['begin_time'] = all_sessions['begin_time'].apply(lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT))
        all_sessions['end_time'] = all_sessions['end_time'].apply(lambda t: kft.strftime(t, kft.SESSION_DATETIME_FORMAT))
        all_sessions['duration'] = all_sessions['duration'].apply(lambda t: kft.strftime(t - kft.DURATION_TZ_ADJUST, kft.DURATION_FORMAT))
        click.echo(cmd.get_help(ctx))
        click.echo('please select sessions:')
        click.echo(tabulate(all_sessions.values, headers=all_sessions.columns, tablefmt='simple'))
        sys.exit()
    else:
        session = kfj.find_session(ctx, session_id)
        instance.set_begin_time(session['begin_time'])
        instance.set_end_time(session['end_time'])
