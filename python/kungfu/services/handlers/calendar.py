
import nnpy, pyyjj

from kungfu.services.handlers import kfs_on

@kfs_on('calendar/current')
def current(ctx, request):
    return {
        "data": {"trading_day":str(ctx._calendar.current_day())}
    }

@kfs_on('calendar/calculate')
def calculate(ctx, request):
    start_date = int(request['start_date'])
    delta = request['delta']
    return {
        "data": {"trading_day":str(ctx._calendar.calculate_trading_day(start_date, delta))}
    }
