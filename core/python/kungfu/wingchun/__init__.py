import pywingchun
import kungfu.yijinjing.time as kft


class Runner(pywingchun.Runner):
    def __init__(self, ctx, mode):
        pywingchun.Runner.__init__(self, ctx.locator, ctx.group, ctx.name, mode, ctx.low_latency)
        self.ctx = ctx
