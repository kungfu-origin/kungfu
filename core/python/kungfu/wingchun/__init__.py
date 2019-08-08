import pywingchun
import kungfu.yijinjing.time as kft


class Runner(pywingchun.Runner):
    def __init__(self, ctx, mode):
        print('*'*100)
        print(ctx.locator, ctx.group, ctx.name, mode, ctx.low_latency)
        print('*'*100)
        pywingchun.Runner.__init__(self, ctx.locator, ctx.group, ctx.name, mode, ctx.low_latency)
        self.ctx = ctx

    def on_trading_day(self, event, daytime):
        self.ctx.logger.info('on switch day %s', kft.to_datetime(daytime))
