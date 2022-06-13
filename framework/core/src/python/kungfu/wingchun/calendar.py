import datetime
import kungfu.yijinjing.time as kft


class Calendar:
    def __init__(self, ctx):
        self.ctx = ctx
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
        return dt.isoweekday() <= 5
