import pyyjj
import psutil
import kungfu.service.kfs as kfs
import kungfu.yijinjing.msg as msg
import kungfu.yijinjing.journal as kfj


@kfs.on(msg.Register)
def register(ctx, event):
    data = event['data']
    mode = kfj.find_mode(data['mode'])
    category = kfj.find_category(data['category'])
    group = data['group']
    name = data['name']
    location = pyyjj.location(mode, category, group, name, ctx.locator)
    pid = data['pid']
    ctx.logger.info('apprentice %s %d checking in', location.uname, pid)
    if pid not in ctx.apprentices:
        ctx.apprentices[pid] = {
            'process': psutil.Process(pid),
            'location': location
        }


@kfs.on(msg.Deregister)
def deregister(ctx, event):
    pid = event['pid']
    ctx.logger.info('apprentice %s checking out', pid)
    ctx.master.deregister_app(event['gen_time'], ctx.apprentices[pid]['location'].uid)


@kfs.task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            ctx.logger.warn('cleaning up stale process pid %s clients %s', pid, ctx.apprentices[pid]['location'].uname)
            ctx.master.deregister_app(pyyjj.now_in_nano(), ctx.apprentices[pid]['location'].uid)
            del ctx.apprentices[pid]
