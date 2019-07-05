
from . import *
from kungfu.data.sqlite.models import *
import pyyjj

def make_url(locator, location, filename):
    db_file = locator.layout_file(location, pyyjj.layout.SQLITE, filename)
    return 'sqlite:///{}'.format(db_file)

class DataProxy:
    def __init__(self, url):
        self.engine = create_engine(url)
        Base.metadata.create_all(self.engine)
        self.session_factory = sessionmaker(bind = self.engine)

    def add_trade(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.add(Trade(**kwargs))

    def add_order(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.merge(Order(**kwargs))

    def save_account(self, account_id, source_id, account_info, positions):
        with session_scope(self.session_factory) as session:
            session.merge(AccountBalance(account_id = account_id, source_id = source_id, **account_info))
            session.query(AccountPosition).filter(AccountPosition.account_id == account_id, AccountPosition.source_id == source_id).delete()
            for pos in positions:
                pos = AccountPosition(account_id = account_id, source_id = source_id, **pos)
                session.add(pos)

    def get_commission(self, account_id, instrument_id, exchange_id):
        pass

    def get_margin_ratio(self, instrument_id, exchange_id, direction):
        pass

    def reset_instruments(self, instruments):
        pass

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

