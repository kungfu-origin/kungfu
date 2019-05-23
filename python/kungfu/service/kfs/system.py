import os
import psutil
import json
import nnpy, pyyjj

import kungfu.service.kfs as kfs
from kungfu.service.kfs.paged import release_client

@kfs.task
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
