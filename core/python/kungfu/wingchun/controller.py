import pywingchun
import pyyjj
import json
import kungfu.yijinjing.time as kft
from kungfu.yijinjing.log import create_logger


class Controller(pywingchun.Controller):
    def __init__(self, ctx):
        mode = pyyjj.mode.REPLAY if ctx.replay else pyyjj.mode.LIVE
        pywingchun.Controller.__init__(self, ctx.locator, mode, ctx.low_latency)
        self.ctx = ctx
        self.ctx.logger = create_logger("controller", ctx.log_level, self.io_device.home)

