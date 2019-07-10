
from . import *
from kungfu.data.sqlite.models import *
import pyyjj
from kungfu.wingchun.constants import *

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

    def get_holidays(self, region = Region.CN):
        with session_scope(self.session_factory) as session:
            return [obj.holiday for obj in session.query(Holiday).filter(Holiday.region == region).all()]

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

class LedgerHolder(DataProxy):
    def __init__(self, url):
        super(LedgerHolder, self).__init__(url)

    def get_model_cls(self, msg_type, ledger_category):
        cls = None
        if msg_type == int(MsgType.AssetInfo):
            if ledger_category == int(LedgerCategory.Account):
                cls = AccountAssetInfo
            elif ledger_category == int(LedgerCategory.Portfolio):
                cls = PortfolioAssetInfo
            elif ledger_category == int(LedgerCategory.SubPortfolio):
                cls = SubPortfolioAssetInfo
        elif msg_type == int(MsgType.Position):
            if ledger_category == int(LedgerCategory.Account):
                cls = AccountPosition
            elif ledger_category == int(LedgerCategory.Portfolio):
                cls =  PortfolioPosition
            elif ledger_category == int(LedgerCategory.SubPortfolio):
                cls = SubPortfolioPosition
        return cls

    def process_message(self, session, message):
        msg_type = message["msg_type"]
        category = message["data"].pop("ledger_category")
        cls = self.get_model_cls(msg_type, category)
        if cls is not None:
            obj = cls(**message["data"])
            session.merge(obj)
            if isinstance(obj, PositionMixin) and obj.volume == 0:
                session.delete(obj)

    def on_messages(self, messages):
        with session_scope(self.session_factory) as session:
            for message in messages:
                self.process_message(session, message)

