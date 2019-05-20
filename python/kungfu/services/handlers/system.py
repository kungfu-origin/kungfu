import os
import psutil
import json
import nnpy, pyyjj

from kungfu.services.handlers import task
from kungfu.services.handlers.paged import release_client

@task
def health_check(ctx):
    stale_pids = []
    for pid in ctx._client_processes:
        if not ctx._client_processes[pid]['process'].is_running():
            ctx._logger.warn('cleaning up stale process pid %s clients %s', pid, ctx._client_processes[pid]['client_info'])
            for name in ctx._client_processes[pid]['client_info']:
                release_client(ctx, name, 0, False, {})
            stale_pids.append(pid)
    for pid in stale_pids:
        del ctx._client_processes[pid]

@task
def switch_trading_day(ctx):
    current_day = ctx._calendar.current_day()
    if ctx._current_day < current_day:
        ctx._current_day = current_day
        ctx._notice_socket.send(json.dumps({
            'type':'calendar',
            'data':{'trading_day':current_day}
        }))
