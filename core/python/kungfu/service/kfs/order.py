import pyyjj
import psutil
import kungfu.service.kfs as kfs
import kungfu.yijinjing.msg as msg
import kungfu.yijinjing.journal as kfj


@kfs.on(msg.UIActionCancelOrder)
def cancel_order(ctx, event):
    data = event['data']


@kfs.on(msg.UIActionCancelAllOrder)
def cancel_all_order(ctx, event):
    data = event['data']

