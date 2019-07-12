
from kungfu.wingchun.constants import *

class TaxCalculator:
    def __init__(self):
        pass

    def calculate(self, trade):
        raise NotImplementationError

class StockTaxCalculator(TaxCalculator):
    def __init__(self, ratio = 0.0001):
        self.ratio = ratio

    def calculate(self, trade):
        return trade.price * trade.volume * self.ratio if trade.side == Side.Sell else 0.0
