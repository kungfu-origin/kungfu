
import nnpy, pyyjj

from kungfu.services.handlers import kfs_on

@kfs_on('calendar/nextday')
def next_trading_day(ctx, request):
    ctx
