
import pywingchun

class Calendar(pywingchun.Calendar):
    def __init__(self, ctx):
        self.ctx = ctx

    def current_trading_day(self):
        return "20190702"
