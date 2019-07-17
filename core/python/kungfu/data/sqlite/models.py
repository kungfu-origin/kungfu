from . import *

Base = declarative_base()

class Task(Base):
    __tablename__ = 'kungfu_task'
    name = Column(String, nullable=False, primary_key=True)
    task_type = Column(String)
    config = Column(Json, nullable=False)

class Holiday(Base):
    __tablename__ = "holidays"
    __table_args__ = (PrimaryKeyConstraint('region', 'holiday'),)
    region = Column(String)
    holiday = Column(Date)

class FutureInstrument(Base):
    __tablename__ = 'future_instrument'
    __table_args__ = (PrimaryKeyConstraint('instrument_id', 'exchange_id'),)
    instrument_id = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(Integer)
    product_id = Column(String)
    contract_multiplier  = Column(Integer)
    price_tick = Column(Float)
    open_date = Column(String)
    create_date = Column(String)
    expire_date = Column(String)
    delivery_year = Column(Integer)
    delivery_month = Column(Integer)
    is_trading = Column(Boolean)
    long_margin_ratio = Column(Float)
    short_margin_ratio = Column(Float)

class Commission(Base):
    __tablename__ = 'commission'
    __table_args__ = (PrimaryKeyConstraint('instrument_id', 'exchange_id', 'instrument_type'),)
    instrument_id = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(Integer)
    account_id = Column(String)
    broker_id = Column(String)
    mode = Column(Integer)
    open_ratio = Column(Float)
    close_ratio = Column(Float)
    close_today_ratio = Column(Float)
    min_commission = Column(Float)

class Order(Base):
    __tablename__ = 'orders'
    order_id = Column(String, primary_key = True)
    insert_time = Column(Integer)
    update_time = Column(Integer)
    trading_day = Column(String)
    instrument_id = Column(String)
    exchange_id = Column(String)
    account_id = Column(String)
    client_id = Column(String)
    instrument_type = Column(Integer)
    limit_price = Column(Float)
    frozen_price = Column(Float)
    volume = Column(Integer)
    volume_traded = Column(Integer)
    volume_left = Column(Integer)
    tax = Column(Float)
    commission = Column(Float)
    status = Column(Integer)
    error_id = Column(Integer)
    error_msg = Column(String)
    side = Column(Integer)
    offset = Column(Integer)
    price_type = Column(Integer)
    volume_condition = Column(Integer)
    time_condition = Column(Integer)
    parent_id = Column(String)


class Trade(Base):
    __tablename__ = 'trades'
    trade_id = Column(String, primary_key = True)
    order_id = Column(String, nullable = False)
    parent_order_id = Column(String)
    trade_time = Column(Integer)
    trading_day = Column(String)
    instrument_id = Column(String)
    exchange_id = Column(String)
    account_id = Column(String)
    client_id = Column(String)
    instrument_type = Column(Integer)
    side = Column(Integer)
    offset = Column(Integer)
    price = Column(Float)
    volume = Column(Integer)
    tax = Column(Float)
    commission = Column(Float)

class LedgerMeta(Base):
    __tablename__ = "ledger_meta"
    uname = Column(String,  primary_key = True)
    update_time = Column(Integer)

class AssetInfoMixin(object):
    trading_day = Column(String)
    update_time = Column(Integer)
    account_id = Column(String)
    source_id = Column(String)
    client_id = Column(String)
    initial_equity = Column(Float)
    static_equity = Column(Float)
    dynamic_equity = Column(Float)
    realized_pnl = Column(Float)
    unrealized_pnl = Column(Float)
    avail = Column(Float)
    market_value = Column(Float)
    margin = Column(Float)
    accumulated_fee = Column(Float)
    intraday_fee = Column(Float)
    frozen_cash = Column(Float)
    frozen_margin = Column(Float)
    frozen_fee = Column(Float)
    position_pnl = Column(Float)
    close_pnl = Column(Float)

    def __init__(self, **kwargs):
        for attr in self.__mapper__.columns.keys():
            if attr in kwargs:
                setattr(self, attr, kwargs[attr])

class AccountAssetInfo(AssetInfoMixin, Base):
    __tablename__ = "account"
    __table_args__ = (PrimaryKeyConstraint('account_id'),)

class PortfolioAssetInfo(AssetInfoMixin, Base):
    __tablename__ = "portfolio"
    __table_args__ = (PrimaryKeyConstraint('client_id'),)

class SubPortfolioAssetInfo(AssetInfoMixin, Base):
    __tablename__ = "subportfolio"
    __table_args__ = (PrimaryKeyConstraint('account_id', 'client_id'),)

class AccountAssetInfoSnapshot(AssetInfoMixin, Base):
    __tablename__ = "account_snapshot"
    __table_args__ = (PrimaryKeyConstraint('account_id', 'update_time'),)

class PortfolioAssetInfoSnapshot(AssetInfoMixin, Base):
    __tablename__ = "portfolio_snapshot"
    __table_args__ = (PrimaryKeyConstraint('client_id', "update_time"),)

class PositionMixin(object):
    trading_day = Column(String)
    account_id = Column(String)
    source_id = Column(String)
    client_id = Column(String)
    instrument_id = Column(String)
    instrument_type = Column(String)
    exchange_id = Column(String)
    direction = Column(Integer)
    volume = Column(Integer)
    yesterday_volume = Column(Integer)
    frozen_total = Column(Integer)
    frozen_yesterday =Column(Integer)
    last_price = Column(Float)
    open_price = Column(Float)
    cost_price = Column(Float)
    close_price = Column(Float)
    pre_close_price = Column(Float)
    settlement_price = Column(Float)
    pre_settlement_price = Column(Float)
    margin = Column(Float)
    position_pnl = Column(Float)
    close_pnl = Column(Float)
    realized_pnl = Column(Float)
    unrealized_pnl = Column(Float)
    open_date = Column(String)
    expire_date = Column(String)

    def __init__(self, **kwargs):
        for attr in self.__mapper__.columns.keys():
            if attr in kwargs:
                setattr(self, attr, kwargs[attr])

class AccountPosition(PositionMixin, Base):
    __tablename__ = "account_position"
    __table_args__ = (PrimaryKeyConstraint('account_id', 'instrument_id', 'exchange_id', 'direction'),)

class PortfolioPosition(PositionMixin, Base):
    __tablename__ = "portfolio_position"
    __table_args__ = (PrimaryKeyConstraint('client_id', 'instrument_id', 'exchange_id', 'direction'),)


class SubPortfolioPosition(PositionMixin, Base):
    __tablename__ = "subportfolio_position"
    __table_args__ = (PrimaryKeyConstraint('account_id', 'client_id','instrument_id', 'exchange_id', 'direction'),)
