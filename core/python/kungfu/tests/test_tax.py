
import unittest
from kungfu.finance.tax import *
from kungfu.wingchun.constants import *
import collections

Trade = collections.namedtuple('Trade', 'price volume side')

class TestTaxCalculator(unittest.TestCase):
    def setUp(self):
        self.calculator = StockTaxCalculator()

    def test_stock_buy_tax(self):
        self.assertEqual(self.calculator.calculate(Trade(price=11.0, volume = 100, side = Side.Buy)), 0.0)

    def test_stock_sell_tax(self):
        self.assertEqual(self.calculator.calculate(Trade(price=11.0, volume = 100, side = Side.Sell)),11.0 * 100 * 0.0001)

if __name__ == '__main__':
    unittest.main()