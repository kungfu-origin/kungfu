import datetime
import collections
import kungfu.yijinjing.time as kft
from kungfu.data.sqlite.data_proxy import DataProxy, make_url
from kungfu.wingchun.constants import *

TradingSession = collections.namedtuple('TradingSession', 'exchange_id trading_day start_time end_time')


class CalendarUtil:
    @classmethod
    def get_trading_sessions(cls, exchange_id, trading_day):
        if exchange_id == Exchange.SSE or exchange_id == Exchange.SZE:
            return [TradingSession(exchange_id = exchange_id,
                                   trading_day = trading_day,
                                   start_time = datetime.datetime.combine(trading_day, datetime.time(9, 30)),
                                   end_time = datetime.datetime.combine(trading_day, datetime.time(11, 30))),
                    TradingSession(exchange_id = exchange_id,
                                   trading_day = trading_day,
                                   start_time = datetime.datetime.combine(trading_day, datetime.time(13, 00)),
                                   end_time = datetime.datetime.combine(trading_day, datetime.time(15, 00))),
                    ]
        else:
            #TODO
            return []


class Calendar:
    def __init__(self, ctx):
        self.holidays = DataProxy(make_url(ctx.locator, ctx.system_config_location, "holidays")).get_holidays()
        self.update_trading_day(datetime.datetime.now())

    def update_trading_day(self, now):
        self.date = now.date()
        if now.hour >= 18:
            self.date = self.date + datetime.timedelta(days=1)
        while not self.is_trading_day(self.date):
            self.date = self.date + datetime.timedelta(days=1)

    @property
    def trading_day(self):
        now = datetime.datetime.now()
        if now.hour >= 18:
            self.update_trading_day(now)
        return self.date

    @property
    def trading_day_ns(self):
        d = self.trading_day
        day = datetime.datetime(year=d.year, month=d.month, day=d.day)
        return int((day - kft.EPOCH).total_seconds() * kft.NANO_PER_SECOND)

    def is_trading_day(self, dt):
        return dt.isoweekday() <= 5 and not self.is_holiday(dt)

    def is_holiday(self, dt):
        return dt in self.holidays


