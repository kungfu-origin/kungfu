import json
import kungfu.service.kfs as kfs
from kungfu.wingchun.constants import *


@kfs.on('calendar/current')
def current(ctx, request):
    return {
        'msg_type': MsgType.RspTradingDay,
        'data': {'trading_day': str(ctx.calendar.current_trading_day())}
    }


@kfs.on('calendar/calculate')
def calculate(ctx, request):
    start_date = int(request['start_date'])
    delta = request['delta']
    return {
        'msg_type': MsgType.RspTradingDay,
        'data': {'trading_day': str(ctx.calendar.calculate_trading_day(start_date, delta))}
    }


@kfs.task
def switch_trading_day(ctx):
    current_trading_day = ctx.calendar.current_trading_day()
    if ctx.current_trading_day < current_trading_day:
        ctx.current_trading_day = current_trading_day
        # ctx._socket_publish.send(json.dumps({
        #     'msg_type': MsgType.SwitchDay,
        #     'data':{'trading_day':current_trading_day}
        # }))
