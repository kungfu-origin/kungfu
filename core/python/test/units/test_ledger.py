import json
import datetime
import time
import unittest
import os
import platform
import http
from kungfu.yijinjing.locator import Locator
from kungfu.yijinjing import msg as yjj_msg
from kungfu.wingchun import msg
from kungfu.wingchun.constants import *

from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj


class LedgerClient:
    def __init__(self, name="tester"):
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
        self.locator = Locator(home)
        self.commander_location = yjj.location(lf.enums.mode.LIVE, lf.enums.category.SYSTEM, 'service', 'ledger', self.locator)
        io_device = yjj.io_device(self.commander_location)
        self.cmd_sock = io_device.connect_socket(self.commander_location, yjj.protocol.REQUEST, 10000)

    def insert_order(self, source_name, account_id, instrument_id, exchange_id, limit_price, volume,
                     price_type=PriceType.Limit, side=Side.Buy, offset=Offset.Open, hedge_flag=HedgeFlag.Speculation):
        data = {"mode": "live", "category": "td", "group": source_name, "name": account_id}
        data.update({"instrument_id": instrument_id, "exchange_id": exchange_id, "limit_price": limit_price, "volume": volume,
                     "price_type": int(price_type), "side": int(side), "offset": int(offset), "hedge_flag": int(hedge_flag)})
        return self._request(msg.NewOrderSingle, data)

    def cancel_order(self, order_id):
        return self._request(msg.CancelOrder, {'mode': 'live', 'order_id': order_id})

    def cancel_all_order_for_account(self, source_name, account_id):
        msg_type = msg.CancelAllOrder
        data = {'mode': 'live', 'category': 'td', 'group': source_name, 'name': account_id}
        return self._request(msg_type, data)

    def cancel_all_order_for_strategy(self, strategy_name):
        msg_type = msg.CancelAllOrder
        data = {'mode': 'live', 'category': 'strategy', 'group': 'default', 'name': strategy_name}
        return self._request(msg_type, data)

    def calendar(self):
        msg_type = msg.Calendar
        return self._request(msg_type)

    def switch_trading_day(self, date):
        if isinstance(date, datetime.date):
            daytime = int(time.mktime(date.timetuple()) * 1e9)
        elif isinstance(date, str):
            daytime = int(time.mktime(datetime.datetime.strptime(date, "%Y%m%d").timetuple()) * 1e9)
        else:
            raise ValueError("invalid date: {}", date)
        return self._request(yjj_msg.TradingDay, {"daytime": daytime})

    def refresh_broker_state(self):
        msg_type = msg.BrokerStateRefresh
        return self._request(msg_type)

    def remove_strategy(self, strategy_name):
        msg_type = msg.RemoveStrategy
        data = {"mode": "live", "category": "strategy", "group": "default", "name": strategy_name}
        return self._request(msg_type, data)

    def asset(self, ledger_category, source_id="", account_id="", client_id=""):
        msg_type = msg.AssetRequest
        data = {"ledger_category": int(ledger_category), "account_id": account_id, "source_id": source_id, "client_id": client_id}
        return self._request(msg_type, data)

    def refresh_all_asset(self):
        return self._request(msg_type=msg.PublishAllAssetInfo)

    def _request(self, msg_type, data={}):
        self.cmd_sock.send(json.dumps({"msg_type": msg_type, "dest": self.commander_location.uid, "data": data}))
        self.cmd_sock.recv()
        return json.loads(self.cmd_sock.last_message())


class TestNewOrderSingle(unittest.TestCase):
    def test_buy(self):
        client = LedgerClient()
        rsp = client.insert_order("xtp", "15040900", "6000000", "SSE", 16.0, 100)
        self.assertEqual(rsp["status"], http.HTTPStatus.OK)
        self.assertGreater(int(rsp["order_id"]), 0)


class TestRemoveStrategy(unittest.TestCase):
    def test_remove_strategy(self):
        client = LedgerClient()
        rsp = client.remove_strategy("test")
        self.assertEqual(rsp["status"], http.HTTPStatus.OK)
