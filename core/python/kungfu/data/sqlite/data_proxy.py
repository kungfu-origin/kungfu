
from . import *
from sqlalchemy import inspect
import datetime
from itertools import groupby
import pyyjj
from kungfu.yijinjing.log import create_logger
from kungfu.data.sqlite.models import *
from kungfu.wingchun.constants import *
from kungfu.wingchun.finance.position import StockPosition, FuturePosition, FuturePositionDetail
from kungfu.wingchun.finance.ledger import *

def make_url(locator, location, filename):
    db_file = locator.layout_file(location, pyyjj.layout.SQLITE, filename)
    return 'sqlite:///{}'.format(db_file)

def object_as_dict(obj):
    return {c.key: getattr(obj, c.key)
            for c in inspect(obj).mapper.column_attrs}

class DataProxy:
    def __init__(self, url, readonly=False):
        self.engine = create_engine(url)
        if not readonly:
            Base.metadata.create_all(self.engine)
        self.session_factory = sessionmaker(bind=self.engine)

    @classmethod
    def setup_logger(cls, level, location):
        create_logger("sqlalchemy.engine", level, location)

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

    def set_instruments(self, instruments):
        with session_scope(self.session_factory) as session:
            session.query(FutureInstrument).delete()
            objects = [FutureInstrument(**inst) for inst in instruments]
            session.bulk_save_objects(objects)

    def get_instrument_info(self, instrument_id):
        with session_scope(self.session_factory) as session:
            obj = session.query(FutureInstrument).filter(FutureInstrument.instrument_id == instrument_id).first()
            return {} if obj is None else object_as_dict(obj)
            
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

    def on_messages(self, messages):
        with session_scope(self.session_factory) as session:
            for message in messages:
                obj = self.object_from_msg(message)
                session.merge(obj)

    def get_stock_positions(self, session, ledger_category, account_id, client_id):
        positions = session.query(Position).filter(Position.account_id == account_id,
                                                   Position.client_id == client_id,
                                                   Position.ledger_category == int(ledger_category),
                                                   Position.instrument_type == int(InstrumentType.Stock)).all()
        return [StockPosition(**object_as_dict(obj)) for obj in positions]

    def get_future_positions(self, session, ledger_category, account_id, client_id):
        objs = session.query(PositionDetail).filter(PositionDetail.account_id == account_id,
                                                    PositionDetail.client_id == client_id,
                                                    PositionDetail.ledger_category == int(ledger_category)
                                                    ).all()
        result = []
        for inst, details in groupby(objs, key= lambda e: e.instrument_id):
            details = [FuturePositionDetail(**object_as_dict(detail)) for detail in sorted(details, key = lambda obj: (obj.open_date, obj.trade_time))]
            inst_info = self.get_instrument_info(inst)
            pos = FuturePosition(instrument_id = inst_info["instrument_id"],
                                 exchange_id = inst_info["exchange_id"],
                                 short_margin_ratio = inst_info["short_margin_ratio"],
                                 long_margin_ratio = inst_info["long_margin_ratio"],
                                 contract_multiplier = inst_info["contract_multiplier"],
                                 details = details,
                                 trading_day = details[0].trading_day)
            result.append(pos)
        return result

    def get_positions(self, session, ledger_category, account_id, client_id):
        return self.get_stock_positions(session, ledger_category, account_id, client_id) + \
               self.get_future_positions(session, ledger_category, account_id, client_id)

    def drop(self, session, ledger_category, account_id="", client_id=""):
        for cls in [Asset, Position, PositionDetail]:
            session.query(cls).filter(cls.account_id == account_id, cls.client_id == client_id,cls.ledger_category == int(ledger_category)).delete()

    def load(self, ledger_category, account_id = "", client_id = ""):
        with session_scope(self.session_factory) as session:
            asset_obj = session.query(Asset).filter(Asset.account_id == account_id,
                                                    Asset.client_id == client_id,
                                                    Asset.ledger_category == int(ledger_category)).first()
            if not asset_obj:
                return None
            else:
                args = object_as_dict(asset_obj)
                positions = self.get_positions(session, ledger_category, account_id, client_id)
                args.update({"positions": positions, "ledger_category": ledger_category})
                return Ledger(ctx = None, **args)

    def dump(self, ledger):
        with session_scope(self.session_factory) as  session:
            self.drop(session, ledger.category, ledger.account_id, ledger.client_id)
            messages = ledger.detail_messages
            objects = [self.object_from_msg(message) for message in messages]
            session.bulk_save_objects(objects)

    def get_model_cls(self, msg_type):
        if msg_type == MsgType.Asset:
            return Asset
        elif msg_type == MsgType.Position:
            return Position
        elif msg_type == MsgType.PositionDetail:
            return PositionDetail
        elif msg_type == MsgType.AssetSnapshot:
            return AssetSnapshot
        else:
            raise ValueError('could not find class for msg_type {}'.format(msg_type))

    def object_from_msg(self, message):
        cls =  self.get_model_cls(message["msg_type"])
        return cls(**message["data"])

