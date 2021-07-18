import unittest
import os
import platform
import datetime
from dotted_dict import DottedDict
import kungfu.yijinjing.journal as kfj
from kungfu.wingchun.book.book import AccountBook
from kungfu.wingchun.constants import *
from kungfu.yijinjing.locator import Locator
from kungfu.yijinjing.log import create_logger
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj


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
        self.ctx.runtime_locator = Locator(home)
        self.ctx.name = "tester"
        self.ctx.log_level = "info"
        self.ctx.now = yjj.now_in_nano
        self.ctx.trading_day = datetime.datetime(2019, 11, 27).date()
        self.ctx.util_location = yjj.location(lf.enums.mode.LIVE, lf.enums.category.SYSTEM, 'util', 'test', self.ctx.runtime_locator)
        self.ctx.logger = create_logger('tester', self.ctx.log_level, self.ctx.util_location)

    def test_buy(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail=10000)

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = lf.enums.Side.Buy
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 200 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0, "incorrect avail")

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 200
        order.limit_price = 16.0
        order.side = lf.enums.Side.Buy
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 200
        trade.price = 16.0
        trade.side = lf.enums.Side.Buy
        trade.offset = lf.enums.Offset.Open
        tax = 0.0
        commission = 200 * 16.0 * 0.0008 if 200 * 16.0 * 0.0008 > 5 else 5
        book.on_trade(event, trade)

        self.assertEqual(trade.commission, commission)
        self.assertEqual(trade.tax, tax)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0 - tax - commission, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission, "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 200, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")

    def test_sell(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dct = {"instrument_id": "600000", "exchange_id": "SSE", "volume": 200, "yesterday_volume": 200, "direction": lf.enums.Direction.Long}
        book = AccountBook(self.ctx, location, avail=10000, positions=[dct])

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = lf.enums.Side.Sell
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000, "incorrect avail")
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 200)
        self.assertEqual(position.frozen_yesterday, 200)

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 200
        order.limit_price = 16.0
        order.side = lf.enums.Side.Sell
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 200
        trade.price = 16.0
        trade.side = lf.enums.Side.Sell
        trade.offset = lf.enums.Offset.Open
        tax = 200 * 16.0 * 0.001
        commission = 5 if 200 * 16.0 * 0.0008 < 5 else 200 * 16.0 * 0.0008
        book.on_trade(event, trade)

        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - tax - commission + trade.volume * trade.price, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission, "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 0, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")

    def test_sell_cancelled(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dct = {"instrument_id": "600000", "exchange_id": "SSE", "volume": 200, "yesterday_volume": 200, "direction": lf.enums.Direction.Long}
        book = AccountBook(self.ctx, location, avail=10000, positions=[dct])

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 1
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = lf.enums.Side.Sell
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000, "incorrect avail")
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 200)
        self.assertEqual(position.frozen_yesterday, 200)

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 1
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 100
        order.volume_left = 100
        order.limit_price = 16.0
        order.side = lf.enums.Side.Sell
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.PartialFilledNotActive
        book.on_order(event, order)
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_total, 100)
        self.assertEqual(position.frozen_yesterday, 100)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.trade_id = 1
        trade.order_id = 1
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 100
        trade.price = 16.0
        trade.side = lf.enums.Side.Sell
        book.on_trade(event, trade)
        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.frozen_yesterday, 0)
        self.assertEqual(position.frozen_total, 0)

    def test_buy_cancelled(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail=10000)

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 100
        input.instrument_id = "600000"
        input.exchange_id = "SSE"
        input.volume = 200
        input.limit_price = 16.0
        input.side = lf.enums.Side.Buy
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)
        self.assertEqual(book.frozen_cash, 200 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 200 * 16.0, "incorrect avail")

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 100
        order.instrument_id = "600000"
        order.exchange_id = "SSE"
        order.volume = 200
        order.volume_traded = 100
        order.volume_left = 100
        order.limit_price = 16.0
        order.side = lf.enums.Side.Buy
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.PartialFilledNotActive

        book.on_order(event, order)

        self.assertEqual(book.frozen_cash, 100 * 16.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - 100 * 16.0, "incorrect avail")

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.trade_id = 1
        trade.order_id = 100
        trade.instrument_id = "600000"
        trade.exchange_id = "SSE"
        trade.volume = 100
        trade.price = 16.0
        trade.side = lf.enums.Side.Buy
        trade.offset = lf.enums.Offset.Open
        tax = 0.0
        commission = trade.volume * trade.price * 0.0008 if trade.volume * trade.price * 0.0008 > 5 else 5

        book.on_trade(event, trade)

        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - trade.price * trade.volume - tax - commission, "incorrect avail")
        self.assertEqual(book.intraday_fee, tax + commission, "incorrect intraday fee")
        self.assertEqual(book.accumulated_fee, tax + commission, "incorrect accumulated fee")

        position = book.get_position(instrument_id="600000", exchange_id="SSE")
        self.assertEqual(position.volume, 100, "incorrect volume")
        self.assertEqual(position.yesterday_volume, 0, "incorrect yesterday volume")


class TestFuturePosition(unittest.TestCase):
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
        self.ctx.runtime_locator = Locator(home)
        self.ctx.name = "tester"
        self.ctx.log_level = "info"
        self.ctx.now = yjj.now_in_nano
        self.ctx.trading_day = datetime.datetime(2019, 11, 27).date()
        self.ctx.util_location = yjj.location(lf.enums.mode.LIVE, lf.enums.category.SYSTEM, 'util', 'test', self.ctx.runtime_locator)
        self.ctx.logger = create_logger('tester', self.ctx.log_level, self.ctx.util_location)
        self.ctx.get_inst_info = lambda instrument_id: {"contract_multiplier": 10, "long_margin_ratio": 0.1, "short_margin_ratio": 0.1}
        self.ctx.get_commission_info = lambda instrument_id: {"mode": 0, "open_ratio": 0.000045, "close_ratio": 0.000045, "close_today_ratio": 0.0}

    def test_open_long(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail=10000)
        dt = datetime.datetime.now()
        try:
            dt = dt.replace(month=dt.month + 1)
        except ValueError:
            if dt.month == 12:
                dt = dt.replace(year=dt.year + 1, month=1)
        instrument_id = "rb" + dt.strftime("%y%m")
        exchange_id = "SHFE"
        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 100
        input.instrument_id = instrument_id
        input.exchange_id = exchange_id
        input.volume = 1
        input.limit_price = 3990.0
        input.side = lf.enums.Side.Buy
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)

        margin_ratio = 0.1
        contract_multiplier = 10
        open_commission_ratio = 0.000045
        close_commission_ratio = 0.000045
        close_today_commission_ratio = 0.0

        margin = margin_ratio * contract_multiplier * input.volume * input.limit_price
        self.assertEqual(book.frozen_margin, margin)
        self.assertEqual(book.frozen_cash, margin, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - margin, "incorrect avail")

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 100
        order.instrument_id = instrument_id
        order.exchange_id = exchange_id
        order.volume = 1
        order.volume_traded = 1
        order.limit_price = 3990.0
        order.side = lf.enums.Side.Buy
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 100
        trade.instrument_id = instrument_id
        trade.exchange_id = exchange_id
        trade.volume = 1
        trade.price = 3990.0
        trade.side = lf.enums.Side.Buy
        trade.offset = lf.enums.Offset.Open
        book.on_trade(event, trade)
        commission = open_commission_ratio * trade.volume * trade.price * contract_multiplier
        tax = 0.0
        self.assertEqual(trade.tax, tax)
        self.assertEqual(trade.commission, commission)
        self.assertEqual(book.frozen_margin, 0.0)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.intraday_fee, tax + commission)
        self.assertEqual(book.accumulated_fee, tax + commission)
        self.assertEqual(book.avail, 10000 - margin - tax - commission, "incorrect avail")

    def test_close_long(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dt = datetime.datetime.now()
        try:
            dt = dt.replace(month=dt.month + 1)
        except ValueError:
            if dt.month == 12:
                dt = dt.replace(year=dt.year + 1, month=1)
        instrument_id = "rb" + dt.strftime("%y%m")
        exchange_id = "SHFE"

        margin_ratio = 0.1
        contract_multiplier = 10
        open_commission_ratio = 0.000045
        close_commission_ratio = 0.000045
        close_today_commission_ratio = 0.0

        dct = {"instrument_id": instrument_id, "exchange_id": exchange_id, "volume": 2, "yesterday_volume": 2,
               "direction": lf.enums.Direction.Long, "margin": margin_ratio * contract_multiplier * 2 * 3980.0}
        book = AccountBook(self.ctx, location, avail=10000, positions=[dct])

        self.assertEqual(book.margin, margin_ratio * contract_multiplier * 2 * 3980.0)

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 100
        input.instrument_id = instrument_id
        input.exchange_id = exchange_id
        input.volume = 1
        input.limit_price = 3990.0
        input.side = lf.enums.Side.Sell
        input.offset = lf.enums.Offset.Close
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)

        position = book.get_position(instrument_id=instrument_id, exchange_id=exchange_id, direction=Direction.Long)
        self.assertEqual(position.frozen_total, 1)
        self.assertEqual(position.frozen_yesterday, 1)

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 100
        order.instrument_id = instrument_id
        order.exchange_id = exchange_id
        order.volume = 1
        order.volume_traded = 1
        order.limit_price = 3990.0
        order.side = lf.enums.Side.Sell
        order.offset = lf.enums.Offset.Close
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 100
        trade.instrument_id = instrument_id
        trade.exchange_id = exchange_id
        trade.volume = 1
        trade.price = 3990.0
        trade.side = lf.enums.Side.Sell
        trade.offset = lf.enums.Offset.Close
        book.on_trade(event, trade)
        margin = trade.price * trade.volume * contract_multiplier * margin_ratio
        commission = close_commission_ratio * trade.volume * trade.price * contract_multiplier
        tax = 0.0
        self.assertEqual(trade.tax, tax)
        self.assertEqual(trade.commission, commission)
        self.assertEqual(book.margin, margin_ratio * contract_multiplier * 2 * 3980.0 - margin)
        self.assertEqual(book.intraday_fee, tax + commission)
        self.assertEqual(book.accumulated_fee, tax + commission)
        self.assertEqual(book.avail, 10000 + margin - tax - commission, "incorrect avail")

    def test_open_short(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        book = AccountBook(self.ctx, location, avail=10000)
        dt = datetime.datetime.now()
        try:
            dt = dt.replace(month=dt.month + 1)
        except ValueError:
            if dt.month == 12:
                dt = dt.replace(year=dt.year + 1, month=1)
        instrument_id = "rb" + dt.strftime("%y%m")
        exchange_id = "SHFE"
        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 100
        input.instrument_id = instrument_id
        input.exchange_id = exchange_id
        input.volume = 1
        input.limit_price = 3990.0
        input.side = lf.enums.Side.Sell
        input.offset = lf.enums.Offset.Open
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)

        margin_ratio = 0.1
        contract_multiplier = 10
        open_commission_ratio = 0.000045
        close_commission_ratio = 0.000045
        close_today_commission_ratio = 0.0

        margin = margin_ratio * contract_multiplier * input.volume * input.limit_price
        self.assertEqual(book.frozen_margin, margin)
        self.assertEqual(book.frozen_cash, margin, 'incorrect frozen cash')
        self.assertEqual(book.avail, 10000 - margin, "incorrect avail")

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 100
        order.instrument_id = instrument_id
        order.exchange_id = exchange_id
        order.volume = 1
        order.volume_traded = 1
        order.limit_price = 3990.0
        order.side = lf.enums.Side.Sell
        order.offset = lf.enums.Offset.Open
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 100
        trade.instrument_id = instrument_id
        trade.exchange_id = exchange_id
        trade.volume = 1
        trade.price = 3990.0
        trade.side = lf.enums.Side.Sell
        trade.offset = lf.enums.Offset.Open
        book.on_trade(event, trade)
        commission = open_commission_ratio * trade.volume * trade.price * contract_multiplier
        tax = 0.0
        self.assertEqual(trade.tax, tax)
        self.assertEqual(trade.commission, commission)
        self.assertEqual(book.frozen_margin, 0.0)
        self.assertEqual(book.frozen_cash, 0.0, 'incorrect frozen cash')
        self.assertEqual(book.intraday_fee, tax + commission)
        self.assertEqual(book.accumulated_fee, tax + commission)
        self.assertEqual(book.avail, 10000 - margin - tax - commission, "incorrect avail")

    def test_close_short(self):
        location = kfj.make_location_from_dict(self.ctx, {"mode": "live", "category": "strategy", "group": "default", "name": "tester"})
        dt = datetime.datetime.now()
        try:
            dt = dt.replace(month=dt.month + 1)
        except ValueError:
            if dt.month == 12:
                dt = dt.replace(year=dt.year + 1, month=1)
        instrument_id = "rb" + dt.strftime("%y%m")
        exchange_id = "SHFE"

        margin_ratio = 0.1
        contract_multiplier = 10
        open_commission_ratio = 0.000045
        close_commission_ratio = 0.000045
        close_today_commission_ratio = 0.0

        dct = {"instrument_id": instrument_id, "exchange_id": exchange_id, "volume": 2, "yesterday_volume": 2,
               "direction": lf.enums.Direction.Short, "margin": margin_ratio * contract_multiplier * 2 * 3980.0}
        book = AccountBook(self.ctx, location, avail=10000, positions=[dct])

        self.assertEqual(book.margin, margin_ratio * contract_multiplier * 2 * 3980.0)

        event = DottedDict()
        event.gen_time = yjj.now_in_nano()
        input = lf.types.OrderInput()
        input.order_id = 100
        input.instrument_id = instrument_id
        input.exchange_id = exchange_id
        input.volume = 1
        input.limit_price = 3990.0
        input.side = lf.enums.Side.Buy
        input.offset = lf.enums.Offset.Close
        input.price_type = lf.enums.PriceType.Limit
        book.on_order_input(event, input)

        position = book.get_position(instrument_id=instrument_id, exchange_id=exchange_id, direction=Direction.Short)
        self.assertEqual(position.frozen_total, 1)
        self.assertEqual(position.frozen_yesterday, 1)

        event.gen_time = yjj.now_in_nano()
        order = lf.types.Order()
        order.order_id = 100
        order.instrument_id = instrument_id
        order.exchange_id = exchange_id
        order.volume = 1
        order.volume_traded = 1
        order.limit_price = 3990.0
        order.side = lf.enums.Side.Buy
        order.offset = lf.enums.Offset.Close
        order.price_type = lf.enums.PriceType.Limit
        order.status = lf.enums.OrderStatus.Filled
        book.on_order(event, order)

        event.gen_time = yjj.now_in_nano()
        trade = lf.types.Trade()
        trade.order_id = 100
        trade.instrument_id = instrument_id
        trade.exchange_id = exchange_id
        trade.volume = 1
        trade.price = 3990.0
        trade.side = lf.enums.Side.Buy
        trade.offset = lf.enums.Offset.Close
        book.on_trade(event, trade)
        margin = trade.price * trade.volume * contract_multiplier * margin_ratio
        commission = close_commission_ratio * trade.volume * trade.price * contract_multiplier
        tax = 0.0
        self.assertEqual(trade.tax, tax)
        self.assertEqual(trade.commission, commission)
        self.assertEqual(book.margin, margin_ratio * contract_multiplier * 2 * 3980.0 - margin)
        self.assertEqual(book.intraday_fee, tax + commission)
        self.assertEqual(book.accumulated_fee, tax + commission)
        self.assertEqual(book.avail, 10000 + margin - tax - commission, "incorrect avail")


if __name__ == '__main__':
    unittest.main()
