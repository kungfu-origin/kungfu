import psutil
import kungfu.service.kfs as kfs


@kfs.on('checkin')
def checkin(ctx, request):
    name = request['name']
    pid = request['pid']
    ctx._logger.info('apprentice %s checking in', pid)
    if pid in ctx._apprentices:
        return {
            'success': False
        }
        # reject
    ctx._apprentices[pid] = {
        'process': psutil.Process(pid),
        'name': name,
        'mm_ids': []
    }
    return {
        'success': True
    }


@kfs.on('checkout')
def checkout(ctx, request):
    pid = request['pid']
    ctx._logger.info('apprentice %s checking out', pid)
    cleanup(ctx, pid)
    return {
        'success': True
    }


@kfs.task
def health_check(ctx):
    for pid in list(ctx._apprentices.keys()):
        if not ctx._apprentices[pid]['process'].is_running():
            ctx._logger.warn('cleaning up stale process pid %s clients %s', pid, ctx._apprentices[pid]['mm_ids'])
            cleanup(ctx, pid)


def cleanup(ctx, pid):
    for mm_id in ctx._apprentices[pid]['mm_ids']:
        ctx._page_service.release_mm_block(mm_id)
    del ctx._apprentices[pid]
