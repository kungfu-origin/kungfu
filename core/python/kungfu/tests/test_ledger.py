

import unittest
from kungfu.finance.ledger import *
from kungfu.finance.position import *
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import *
import datetime
import collections

Trade = collections.namedtuple('Trade', 'price volume side instrument_id, exchange_id')

class TestStockLedger(unittest.TestCase):
    def test_init_ledger(self):
        position = StockPosition(instrument_id = "601988", exchange_id = "SSE", open_price = 3.9, pre_close_price = 3.85, close_price = 0.0, last_price = 0.0, volume = 200, yesterday_volume = 200)
        self.assertEqual(position.unrealized_pnl, 0.0)
        self.assertEqual(position.realized_pnl, 0.0)
        ledger = Ledger(trading_day = datetime.datetime(2017, 9, 1), avail = 100000.0, positions = {get_symbol_id(position.instrument_id, position.exchange_id): position})
        self.assertEqual(ledger.market_value, 3.9 * 200 )
        self.assertEqual(ledger.dynamic_equity, 3.9 * 200 + 100000.0)

    def test_stock_buy(self):
        position = StockPosition(instrument_id = "601988", exchange_id = "SSE", open_price = 3.9, pre_close_price = 3.85, close_price = 0.0, last_price = 0.0, volume = 200, yesterday_volume = 200)
        ledger = Ledger(trading_day = datetime.datetime(2017, 9, 1), avail = 100000.0, positions = {get_symbol_id(position.instrument_id, position.exchange_id): position})
        ledger.apply_trade(Trade(instrument_id = "601988", exchange_id = "SSE", price = 3.8, volume = 100, side = Side.Buy))
        position = ledger._get_position(instrument_id = "601988", exchange_id = "SSE")
        self.assertEqual(position.avg_open_price, (3.9 * 200 + 3.8 * 100) / 300)

    def test_stock_sell(self):
        pass

    def test_switch_day(self):
        pass

if __name__ == '__main__':
    unittest.main()