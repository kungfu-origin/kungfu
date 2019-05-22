
import nnpy, pyyjj
import kungfu.service.kfs as kfs

@kfs.on('calendar/current')
def current(ctx, request):
    return {
        "data": {"trading_day":str(ctx._calendar.current_day())}
    }

@kfs.on('calendar/calculate')
def calculate(ctx, request):
    start_date = int(request['start_date'])
    delta = request['delta']
    return {
        "data": {"trading_day":str(ctx._calendar.calculate_trading_day(start_date, delta))}
    }
