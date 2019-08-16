import json
from . import *
from .models import *
from itertools import groupby
from kungfu.wingchun.finance.position import StockPosition, FuturePosition, FuturePositionDetail
from kungfu.wingchun.finance.book import *

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker


class SessionFactoryHolder:
    def __init__(self, location, filename):
        self.engine = create_engine(make_url(location, filename))
        self.session_factory = sessionmaker(bind=self.engine)


class AccountsDB(SessionFactoryHolder):
    def __init__(self, location, filename):
        super(AccountsDB, self).__init__(location, filename)
        Account.metadata.create_all(self.engine)

    def get_accounts(self):
        with session_scope(self.session_factory) as session:
            return [object_as_dict(obj) for obj in session.query(Account).all()]

    def get_td_account_config(self, source_name, account_id):
        with session_scope(self.session_factory) as session:
            account = session.query(Account).filter(Account.source_name == source_name and Account.account_id == account_id).first()
            return json.dumps(object_as_dict(account)['config'])

    def get_md_account_config(self, source_name):
        with session_scope(self.session_factory) as session:
            account = session.query(Account).filter(Account.source_name == source_name and Account.receive_md).first()
            return json.dumps(object_as_dict(account)['config'])


class CalendarDB(SessionFactoryHolder):
    def __init__(self, location, filename):
        super(CalendarDB, self).__init__(location, filename)
        Holiday.metadata.create_all(self.engine)

    def get_holidays(self, region=Region.CN):
        with session_scope(self.session_factory) as session:
            return [obj.holiday for obj in session.query(Holiday).filter(Holiday.region == region).all()]


class LedgerDB(SessionFactoryHolder):
    def __init__(self, location, filename):
        super(LedgerDB, self).__init__(location, filename)
        Base.metadata.create_all(self.engine)

    def on_messages(self, messages):
        with session_scope(self.session_factory) as session:
            for message in messages:
                obj = self.object_from_msg(message)
                session.merge(obj)

    def add_trade(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.add(Trade(**kwargs))

    def add_order(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.merge(Order(**kwargs))

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

