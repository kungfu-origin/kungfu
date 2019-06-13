import psutil
import kungfu.service.kfs as kfs


@kfs.on('checkin')
def checkin(ctx, request):
    name = request['name']
    pid = request['pid']
    ctx.logger.info('apprentice %s checking in', pid)
    if pid in ctx.apprentices:
        return {
            'success': False
        }
        # reject
    ctx.apprentices[pid] = {
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
    ctx.logger.info('apprentice %s checking out', pid)
    cleanup(ctx, pid)
    return {
        'success': True
    }


@kfs.task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            ctx.logger.warn('cleaning up stale process pid %s clients %s', pid, ctx.apprentices[pid]['mm_ids'])
            cleanup(ctx, pid)


def cleanup(ctx, pid):
    for mm_id in ctx.apprentices[pid]['mm_ids']:
        ctx.page_service.release_mm_block(mm_id)
    del ctx.apprentices[pid]
