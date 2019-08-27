import json

from .models import *
from itertools import groupby
from kungfu.wingchun.finance.position import StockPosition, FuturePosition, FuturePositionDetail
from kungfu.wingchun.finance.position import get_uid as get_position_uid
from kungfu.wingchun.finance.book import *
from kungfu.wingchun.constants import *
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import not_
from . import object_as_dict
from . import make_url

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

    def find_account(self, account_id):
        with session_scope(self.session_factory) as session:
            account = session.query(Account).filter(Account.account_id == account_id).first()
            return object_as_dict(account)

    def list_source_accounts(self, source_name):
        with session_scope(self.session_factory) as session:
            accounts = session.query(Account).filter(Account.source_name == source_name)
            return [object_as_dict(obj) for obj in accounts]

    def get_td_account_config(self, source_name, account_id):
        with session_scope(self.session_factory) as session:
            account = session.query(Account).\
                filter(Account.source_name == source_name).\
                filter(Account.account_id == account_id).first()
            return json.dumps(object_as_dict(account)['config'])

    def get_md_account_config(self, source_name):
        with session_scope(self.session_factory) as session:
            account = session.query(Account).\
                filter(Account.source_name == source_name).\
                filter(Account.receive_md).first()
            return json.dumps(object_as_dict(account)['config'])

    def reset_receive_md(self):
        with session_scope(self.session_factory) as session:
            for obj in session.query(Account):
                obj.receive_md = False
                session.merge(obj)

    def add_account(self, **kwargs):
        with session_scope(self.session_factory) as session:
            session.merge(Account(**kwargs))

    def delete_account(self, account_id):
        with session_scope(self.session_factory) as session:
            session.query(Account).filter(Account.account_id == account_id).delete()


class CalendarDB(SessionFactoryHolder):
    def __init__(self, location, filename):
        super(CalendarDB, self).__init__(location, filename)

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

    def mark_orders_status_unknown(self, source_id, account_id):
        with session_scope(self.session_factory) as session:
            pending_orders = session.query(Order).filter(Order.account_id == account_id, not_(Order.status.in_(AllFinalOrderStatus))).all()
            for order in pending_orders:
                order.status = int(OrderStatus.Unknown)
            return [object_as_dict(order) for order in pending_orders]

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

    def all_instrument_infos(self):
        with session_scope(self.session_factory) as session:
            objs = session.query(FutureInstrument).all()
            return [object_as_dict(obj) for obj in objs]

    def drop(self, session, ledger_category, source_id="",account_id="", client_id=""):
        for cls in [Asset, Position, PositionDetail]:
            session.query(cls).filter(cls.source_id==source_id, cls.account_id == account_id, cls.client_id == client_id,cls.ledger_category == int(ledger_category)).delete()

    def load(self, ledger_category, source_id="",account_id = "", client_id = ""):
        with session_scope(self.session_factory) as session:
            asset_obj = session.query(Asset).filter(Asset.source_id==source_id,
                                                    Asset.account_id == account_id,
                                                    Asset.client_id == client_id,
                                                    Asset.ledger_category == int(ledger_category)).first()
            if not asset_obj:
                return None
            else:
                args = object_as_dict(asset_obj)
                pos_objs = session.query(Position).filter(Position.source_id==source_id,
                                                          Position.account_id == account_id,
                                                          Position.client_id == client_id,
                                                          Position.ledger_category == int(ledger_category)).all()
                detail_objs = session.query(PositionDetail).filter(PositionDetail.source_id==source_id,
                                                                   PositionDetail.account_id == account_id,
                                                                   PositionDetail.client_id == client_id,
                                                                   PositionDetail.ledger_category == int(ledger_category)).all()
                pos_dict = {get_position_uid(pos.instrument_id, pos.exchange_id, pos.direction): pos for pos in pos_objs}
                positions = []
                for uid, pos in pos_dict.items():
                    if pos.instrument_type == int(InstrumentType.Stock):
                        positions.append(StockPosition(**object_as_dict(pos)))
                for uid, details in groupby(detail_objs, key= lambda e: get_position_uid(e.instrument_id, e.exchange_id, e.direction)):
                    details = [FuturePositionDetail(**object_as_dict(detail)) for detail in sorted(details, key = lambda obj: (obj.open_date, obj.trade_time))]
                    summary = pos_dict[uid]
                    pos = FuturePosition(instrument_id = summary.instrument_id,
                                         exchange_id = summary.exchange_id,
                                         margin_ratio = summary.margin_ratio,
                                         contract_multiplier = summary.contract_multiplier,
                                         realized_pnl = summary.realized_pnl,
                                         details = details,
                                         trading_day = details[0].trading_day)
                    positions.append(pos)
                args.update({"positions": positions, "ledger_category": ledger_category})
                return AccountBook(ctx=None, **args)

    def dump(self, ledger):
        with session_scope(self.session_factory) as session:
            self.drop(session, ledger.category, source_id=ledger.source_id, account_id=ledger.account_id, client_id=ledger.client_id)
            messages = ledger.detail_messages
            objects = [self.object_from_msg(message) for message in messages]
            session.bulk_save_objects(objects)

    def remove(self, ledger):
        with session_scope(self.session_factory) as session:
            self.drop(session, ledger.category, source_id=ledger.source_id, account_id=ledger.account_id, client_id=ledger.client_id)

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

