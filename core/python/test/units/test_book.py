import unittest
import os
import platform
import datetime
from dotted_dict import DottedDict
import pyyjj
import pywingchun
from kungfu.wingchun.book.book import AccountBook
import kungfu.yijinjing.journal as kfj
import kungfu.yijinjing.time as kft
from kungfu.yijinjing.log import create_logger

class TestStockPosition(unittest.TestCase):
    def setUp(self):
        self.ctx = DottedDict()
        osname = platform.system()
        user_home = os.path.expanduser('~')
        if osname == 'Linux':
            xdg_config_home = os.getenv('XDG_CONFIG_HOME')
            home = xdg_config_home if xdg_config_home else os.path.join(user_home, '.config')
        if osname == 'Darwin':
            home = os.path.join(user_home, 'Library', 'Application Support')
        if osname == 'Windows':
            home = os.getenv('APPDATA')
        home = os.path.join(home, 'kungfu', 'app')
        self.ctx.locator = kfj.Locator(home)
        self.ctx.name = "tester"
        self.ctx.log_level = "info"
        self.ctx.now = pyyjj.now_in_nano
        self.ctx.trading_day = datetime.datetime(2019, 11, 27).date()
        self.ctx.util_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'util', 'test', self.ctx.locator)
        self.ctx.logger = create_logger('tester', self.ctx.log_level, self.ctx.util_location)

    def test_buy(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail = 10000)

        event = DottedDict()
        event.gen_time = pyyjj.now_in_nano()
        input = pywingchun.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = pywingchun.constants.Side.Buy
        input.offset = pywingchun.constants.Offset.Open
        input.price_type = pywingchun.constants.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 200 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0, "incorrect avail")

        event.gen_time = pyyjj.now_in_nano()
        order = pywingchun.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 200
        order.limit_price = 16.0
        order.side = pywingchun.constants.Side.Buy
        order.offset = pywingchun.constants.Offset.Open
        order.price_type = pywingchun.constants.PriceType.Limit
        order.status = pywingchun.constants.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = pyyjj.now_in_nano()
        trade = pywingchun.Trade()
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 200
        trade.price = 16.0
        trade.side = pywingchun.constants.Side.Buy
        trade.offset = pywingchun.constants.Offset.Open
        tax = 0.0
        commission = 200 * 16.0 * 0.0008 if 200 * 16.0 * 0.0008 > 5 else 5
        book.on_trade(event, trade)

        self.assertEqual(trade.commission, commission)
        self.assertEqual(trade.tax, tax)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0 - tax - commission, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission,  "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 200, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")

    def test_sell(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dct = {"instrument_id": "600000", "exchange_id": "SSE", "volume": 200, "yesterday_volume": 200, "direction": pywingchun.constants.Direction.Long}
        book = AccountBook(self.ctx, location, avail = 10000, positions= [dct])

        event = DottedDict()
        event.gen_time = pyyjj.now_in_nano()
        input = pywingchun.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = pywingchun.constants.Side.Sell
        input.offset = pywingchun.constants.Offset.Open
        input.price_type = pywingchun.constants.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000, "incorrect avail")
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 200)
        self.assertEqual(position.frozen_yesterday, 200)

        event.gen_time = pyyjj.now_in_nano()
        order = pywingchun.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 200
        order.limit_price = 16.0
        order.side = pywingchun.constants.Side.Sell
        order.offset = pywingchun.constants.Offset.Open
        order.price_type = pywingchun.constants.PriceType.Limit
        order.status = pywingchun.constants.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = pyyjj.now_in_nano()
        trade = pywingchun.Trade()
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 200
        trade.price = 16.0
        trade.side = pywingchun.constants.Side.Sell
        trade.offset = pywingchun.constants.Offset.Open
        tax = 200 * 16.0 * 0.001
        commission = 5 if 200 * 16.0 * 0.0008 < 5 else 200 * 16.0 * 0.0008
        book.on_trade(event, trade)

        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - tax - commission + trade.volume * trade.price, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission,  "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 0, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")

    def test_sell_cancelled(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dct = {"instrument_id": "600000", "exchange_id": "SSE", "volume": 200, "yesterday_volume": 200, "direction": pywingchun.constants.Direction.Long}
        book = AccountBook(self.ctx, location, avail = 10000, positions= [dct])

        event = DottedDict()
        event.gen_time = pyyjj.now_in_nano()
        input = pywingchun.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = pywingchun.constants.Side.Sell
        input.offset = pywingchun.constants.Offset.Open
        input.price_type = pywingchun.constants.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000, "incorrect avail")
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 200)
        self.assertEqual(position.frozen_yesterday, 200)

        event.gen_time = pyyjj.now_in_nano()
        order = pywingchun.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 100
        order.volume_left = 100
        order.limit_price = 16.0
        order.side = pywingchun.constants.Side.Sell
        order.offset = pywingchun.constants.Offset.Open
        order.price_type = pywingchun.constants.PriceType.Limit
        order.status = pywingchun.constants.OrderStatus.PartialFilledNotActive
        book.on_order(event, order)
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 100)
        self.assertEqual(position.frozen_yesterday, 100)

        event.gen_time = pyyjj.now_in_nano()
        trade = pywingchun.Trade()
        trade.trade_id = 1
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 100
        trade.price = 16.0
        trade.side = pywingchun.constants.Side.Sell
        book.on_trade(event, trade)
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_yesterday, 0)
        self.assertEqual(position.frozen_total, 0)

    def test_buy_cancelled(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail = 10000)

        event = DottedDict()
        event.gen_time = pyyjj.now_in_nano()
        input = pywingchun.OrderInput()
        input.order_id = 100
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = pywingchun.constants.Side.Buy
        input.offset = pywingchun.constants.Offset.Open
        input.price_type = pywingchun.constants.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 200 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0, "incorrect avail")

        event.gen_time = pyyjj.now_in_nano()
        order = pywingchun.Order()
        order.order_id = 100
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 100
        order.volume_left = 100
        order.limit_price = 16.0
        order.side = pywingchun.constants.Side.Buy
        order.offset = pywingchun.constants.Offset.Open
        order.price_type = pywingchun.constants.PriceType.Limit
        order.status = pywingchun.constants.OrderStatus.PartialFilledNotActive

        book.on_order(event, order)

        self.assertEqual(book.frozen_cash, 100 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 100 * 16.0, "incorrect avail")

        event.gen_time = pyyjj.now_in_nano()
        trade = pywingchun.Trade()
        trade.trade_id = 1
        trade.order_id = 100
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 100
        trade.price = 16.0
        trade.side = pywingchun.constants.Side.Buy
        trade.offset = pywingchun.constants.Offset.Open
        tax = 0.0
        commission = trade.volume * trade.price * 0.0008 if trade.volume * trade.price * 0.0008 > 5 else 5

        book.on_trade(event, trade)

        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - trade.price * trade.volume - tax - commission, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission,  "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 100, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")


class TestFuturePosition(unittest.TestCase):
    def test_open_long(self):
        pass

    def test_close_long(self):
        pass

    def test_open_short(self):
        pass

    def test_close_short(self):
        pass

if __name__ == '__main__':
    unittest.main()