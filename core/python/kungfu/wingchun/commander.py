import pywingchun
import pyyjj
import json
import kungfu.yijinjing.time as kft
from kungfu.yijinjing.log import create_logger


class Commander(pywingchun.Commander):
    def __init__(self, ctx):
        pywingchun.Commander.__init__(self, ctx.locator, ctx.low_latency, ctx.name)
        self.ctx = ctx
        self.ctx.logger = create_logger(ctx.name, ctx.log_level, self.io_device.home)

