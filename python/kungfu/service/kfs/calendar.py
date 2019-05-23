
import nnpy, pyyjj
import kungfu.service.kfs as kfs
from kungfu.wingchun.constants import *

@kfs.on('calendar/current')
def current(ctx, request):
    return {
        'msg_type': MsgType.RspTradingDay,
        'data': {'trading_day':str(ctx._calendar.current_day())}
    }

@kfs.on('calendar/calculate')
def calculate(ctx, request):
    start_date = int(request['start_date'])
    delta = request['delta']
    return {
        'msg_type': MsgType.RspTradingDay,
        'data': {'trading_day':str(ctx._calendar.calculate_trading_day(start_date, delta))}
    }

@kfs.task
def switch_trading_day(ctx):
    current_day = ctx._calendar.current_day()
    if ctx._current_day < current_day:
        ctx._current_day = current_day
        ctx._notice_socket.send(json.dumps({
            'msg_type': MsgType.SwitchDay,
            'data':{'trading_day':current_day}
        }))
