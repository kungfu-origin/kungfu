import json
import kungfu.service.kfs as kfs
from kungfu.wingchun.constants import *


@kfs.on('calendar/current')
def current(ctx, request):
    return {
        'msg_type': MsgType.RspTradingDay,
        'data': {'trading_day': str(ctx.calendar.trading_day)}
    }

@kfs.task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        #TODO publish trading day changed
