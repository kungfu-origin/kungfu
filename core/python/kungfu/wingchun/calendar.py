
from kungfu.data.sqlite.data_proxy import DataProxy, make_url
import datetime

class Calendar:
    def __init__(self, ctx):
        self.holidays = DataProxy(make_url(ctx.locator, ctx.system_config_location, "holidays")).get_holidays()

    @property
    def trading_day(self):
        now = datetime.datetime.now()
        date = now.date()
        if now.hour >= 18:
            date = date + datetime.timedelta(days =1)
        while True:
            if self.is_trading_day(date):
                break
            else:
                date = date + datetime.timedelta(days = 1)
        return date

    def is_trading_day(self, dt):
        return dt.isoweekday() < 5 and not self.is_holiday(dt)

    def is_holiday(self, dt):
        return dt in self.holidays


