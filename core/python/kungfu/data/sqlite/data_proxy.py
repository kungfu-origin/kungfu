
from . import *
from kungfu.data.sqlite.models import *
import pyyjj

def make_url(locator, location, filename):
    db_file = locator.layout_file(location, pyyjj.layout.SQLITE, filename)
    return 'sqlite:///{}'.format(db_file)


class Storage:
    def __init__(self, url):
        self.engine = create_engine(url)
        Base.metadata.create_all(self.engine)
        self.session_factory = sessionmaker(bind = self.engine)

class OrderStorage(Storage):
    def __init__(self, url):
        super(OrderStorage, self).__init__(url)

    def add_order(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.merge(Order(**kwargs))

class TradeStorage(Storage):
    def __init__(self, url):
        super(TradeStorage, self).__init__(url)

    def add_trade(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.add(Trade(**kwargs))

class FutureInstrumentStorage(Storage):
    def __init__(self, url):
       super(FutureInstrumentStorage, self).__init__(url)

    def get_instrument(self, instrument_id, exchange_id):
        return None

    def reset_instruments(self, instruments):
        pass

class CommissionInfoStorage:
    def __init__(self, location):
        pass

class TaskConfigStorage(Storage):
    def __init__(self, url):
        super(TaskConfigStorage, self).__init__(url)

    def get_task_config(self, task_name):
        with session_scope(self.session_factory) as session:
            task = session.query(Task).get(task_name)
            if task:
                return task.config
            else:
                return {}

    def set_task_config(self, task_name, config):
        with session_scope(self.session_factory) as session:
            task = session.query(Task).get(task_name)
            if task is None:
                task = Task(name=task_name, config=config)
                session.add(task)
            else:
                task.config = config

class DataProxy:
    def __init__(self, ctx):
        self.ctx = ctx
        ctx.data_proxy = self
        default_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator)
        watcher_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'watcher', 'watcher', ctx.locator)
        self._order_storage = OrderStorage(make_url(ctx.locator, watcher_location, "orders"))
        self._trade_storage = TradeStorage(make_url(ctx.locator, watcher_location, "trades"))
        self._task_storage = TaskConfigStorage(make_url(ctx.locator,default_location, "task"))
        self._instrument_storage = FutureInstrumentStorage(make_url(ctx.locator, default_location, "future_instruments"))

    def add_order(self, **kwargs):
        self._order_storage.add_order(**kwargs)

    def add_trade(self, **kwargs):
        self._trade_storage.add_trade(**kwargs)

    def get_commission(self, account_id, instrument_id, exchange_id):
        pass

    def get_margin_ratio(self, instrument_id, exchange_id, direction):
        pass

    def reset_instruments(self, instruments):
        pass

    def get_task_config(self, task_name):
        return self._task_storage.get_task_config(task_name)

    def set_task_config(self, task_name, config):
        pass

