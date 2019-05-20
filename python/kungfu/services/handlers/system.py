import os
import psutil
import nnpy, pyyjj

from kungfu.services.handlers import kfs_task
from kungfu.services.handlers.paged import release_client

@kfs_task
def health_check(ctx):
    stale_pids = []
    for pid in ctx.client_processes:
        if not ctx.client_processes[pid]['process'].is_running():
            for name in ctx.client_processes[pid]['client_info']:
                release_client(ctx, name, 0, False, {})
            stale_pids.append(pid)
    for pid in stale_pids:
        del ctx.client_processes[pid]