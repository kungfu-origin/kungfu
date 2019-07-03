
from . import *

AssetBase = declarative_base()


class AssetStorage:
    def __init__(self, category, name):
        self.category = category
        self.name = name
        self.engine = create_engine('sqlite:///test.db')
        AssetBase.metadata.create_all(self.engine)
        self.session_factory = sessionmaker(bind= self.engine)

    def save(self, asset):
        with session_scope(self.session_factory) as session:
            session.query(Portfolio).delete()
            session.query(Account).delete()
            session.query(Position).delete()
            session.query(PositionDetail).delete()
            session.query(FrozenPosition).delete()
            session.query(FrozenCash).delete()
            for acc in asset.accounts:
                obj = Account(account_id = acc.account_id)
                session.add(obj)
            for pos in asset.positions:
                obj = Position(instrument_id = pos.instrument_id, exchange_id = pos.exchange_id, account_id = pos.account_id, client_id = pos.client_id, direction = pos.direction)
                session.add(obj)
            for frozen_pos in asset.position_frozens:
                obj = FrozenPosition(operation_id = frozen_pos.operation_Id)
                session.add(obj)
            for frozen_cash in asset.cash_frozens:
                obj = FrozenCash(operation_id = frozen_cash.operation_id)
                session.add(obj)
            meta = AssetMeta(category = self.category, name = self.name, update_time = asset.update_time, trading_day = asset.trading_day)
            session.add(meta)

    def load(self):
        snapshot = pywingchun.AssetSnapshot()
        snapshot.last_update = 0
        snapshot.trading_day = ""
        return snapshot



