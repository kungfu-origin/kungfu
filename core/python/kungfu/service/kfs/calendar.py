
import kungfu.service.kfs as kfs
import kungfu.yijinjing.msg as yjj_msg

@kfs.task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        ctx.master.publish_time(yjj_msg.TradingDay, ctx.calendar.trading_day_ns)
