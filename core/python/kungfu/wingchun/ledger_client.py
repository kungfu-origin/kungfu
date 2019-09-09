
import pyyjj
import json
from kungfu.wingchun import msg
import kungfu.yijinjing.msg as yjj_msg
import datetime
import time

class LedgerClient:
    def __init__(self, locator, name="ledger_client"):
        self.commander_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'service', 'ledger', locator)
        io_device = pyyjj.io_device(self.commander_location)
        self.cmd_sock = io_device.connect_socket(self.commander_location, pyyjj.protocol.REQUEST, 10000)

    def cancel_order(self, order_id):
        return self._request(msg.CancelOrder, {'mode': 'live','order_id': order_id})

    def cancel_all_order_for_account(self, source_name, account_id):
        msg_type = msg.CancelAllOrder
        data = {'mode':'live','category':'td','group': source_name,'name': account_id}
        return self._request(msg_type, data)

    def cancel_all_order_for_strategy(self, strategy_name):
        msg_type = msg.CancelAllOrder
        data =  {'mode': 'live','category': 'strategy','group': 'default','name': strategy_name}
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
        msg_type = msg.QryAsset
        data = {"ledger_category": int(ledger_category), "account_id": account_id, "source_id": source_id, "client_id":client_id}
        return self._request(msg_type, data)

    def refresh_all_asset(self):
        return self._request(msg_type=msg.PublishAllAssetInfo)

    def _request(self, msg_type, data = {}):
        self.cmd_sock.send(json.dumps({"msg_type": msg_type, "dest": self.commander_location.uid, "data": data}))
        self.cmd_sock.recv()
        return self.cmd_sock.last_message()
