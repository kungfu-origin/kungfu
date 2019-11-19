from . import *

from sqlalchemy import Column, ForeignKey, Integer, String, Date, Float, Boolean, PrimaryKeyConstraint
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship

Base = declarative_base()

class ModelMixin(object):
    def __init__(self, **kwargs):
        for attr in self.__mapper__.columns.keys():
            if attr in kwargs:
                setattr(self, attr, kwargs[attr])

class Account(Base):
    __tablename__ = 'account_config'
    account_id = Column(String, nullable=False, primary_key=True)
    source_name = Column(String)
    receive_md = Column(Boolean)
    config = Column(Json, nullable=False)

class Holiday(Base):
    __tablename__ = "holidays"
    __table_args__ = (PrimaryKeyConstraint('region', 'holiday'),)
    region = Column(String)
    holiday = Column(Date)

class FutureInstrument(ModelMixin, Base):
    __tablename__ = 'future_instrument'
    __table_args__ = (PrimaryKeyConstraint('instrument_id', 'exchange_id'),)
    instrument_id = Column(String)
    exchange_id = Column(String)
    instrument_type = Column(InstrumentType)
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
    instrument_type = Column(InstrumentType)
    account_id = Column(String)
    broker_id = Column(String)
    mode = Column(Integer)
    open_ratio = Column(Float)
    close_ratio = Column(Float)
    close_today_ratio = Column(Float)
    min_commission = Column(Float)

class Order(ModelMixin, Base):
    __tablename__ = 'orders'
    order_id = Column(UINT64, primary_key = True)
    insert_time = Column(Integer)
    update_time = Column(Integer)
    trading_day = Column(String)
    instrument_id = Column(String)
    exchange_id = Column(String)
    source_id = Column(String)
    account_id = Column(String)
    client_id = Column(String)
    instrument_type = Column(InstrumentType)
    limit_price = Column(Float)
    frozen_price = Column(Float)
    volume = Column(Integer)
    volume_traded = Column(Integer)
    volume_left = Column(Integer)
    tax = Column(Float)
    commission = Column(Float)
    status = Column(OrderStatus)
    error_id = Column(Integer)
    error_msg = Column(String)
    side = Column(Side)
    offset = Column(Offset)
    hedge_flag = Column(HedgeFlag)
    price_type = Column(PriceType)
    volume_condition = Column(VolumeCondition)
    time_condition = Column(VolumeCondition)
    parent_id = Column(UINT64)

class Trade(ModelMixin, Base):
    __tablename__ = 'trades'
    trade_id = Column(UINT64, primary_key = True)
    order_id = Column(UINT64, nullable = False)
    parent_order_id = Column(UINT64)
    trade_time = Column(Integer)
    trading_day = Column(String)
    instrument_id = Column(String)
    exchange_id = Column(String)
    source_id = Column(String)
    account_id = Column(String)
    client_id = Column(String)
    instrument_type = Column(InstrumentType)
    side = Column(Side)
    offset = Column(Offset)
    hedge_flag = Column(HedgeFlag)
    price = Column(Float)
    volume = Column(Integer)
    tax = Column(Float)
    commission = Column(Float)

class AssetMixin(ModelMixin):
    trading_day = Column(String)
    update_time = Column(Integer)
    holder_uid = Column(Integer)
    ledger_category = Column(LedgerCategory)
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

class Asset(AssetMixin, Base):
    __tablename__ = "asset"
    __table_args__ = (PrimaryKeyConstraint('holder_uid'),)

class AssetSnapshot(AssetMixin, Base):
    __tablename__ = "asset_snapshot"
    __table_args__ = (PrimaryKeyConstraint("holder_uid", 'update_time'),)

class Position(ModelMixin, Base):
    __tablename__ = "position"
    id = Column(UINT64, primary_key = True)
    update_time = Column(Integer)
    trading_day = Column(String)

    instrument_id = Column(String)
    instrument_type = Column(InstrumentType)
    exchange_id = Column(String)

    holder_uid = Column(Integer)
    ledger_category = Column(LedgerCategory)
    account_id = Column(String)
    source_id = Column(String)
    client_id = Column(String)

    direction = Column(Direction)
    volume = Column(Integer)
    yesterday_volume = Column(Integer)
    frozen_total = Column(Integer)
    frozen_yesterday =Column(Integer)

    last_price = Column(Float)
    avg_open_price = Column(Float)
    position_cost_price = Column(Float)

    close_price = Column(Float)
    pre_close_price = Column(Float)

    settlement_price = Column(Float)
    pre_settlement_price = Column(Float)

    margin = Column(Float)
    position_pnl = Column(Float)
    close_pnl = Column(Float)

    realized_pnl = Column(Float)
    unrealized_pnl = Column(Float)

    margin_ratio = Column(Float)
    contract_multiplier = Column(Integer)

    def __init__(self, **kwargs):
        super(Position, self).__init__(**kwargs)
        self.id = self.holder_uid << 32 ^ pyyjj.hash_str_32("{}.{}.{}".format(self.instrument_id, self.exchange_id, self.direction))

class PositionDetail(ModelMixin, Base):
    __tablename__ = "position_detail"
    __table_args__ = (PrimaryKeyConstraint('holder_uid', 'trade_id'),)

    update_time = Column(Integer)
    trading_day = Column(String)

    instrument_id = Column(String)
    instrument_type = Column(String)
    exchange_id = Column(String)

    holder_uid = Column(Integer)
    account_id = Column(String)
    source_id = Column(String)
    client_id = Column(String)

    direction = Column(Direction)

    volume = Column(Integer)
    frozen_volume = Column(Integer)

    last_price = Column(Float)
    open_price = Column(Float)

    settlement_price = Column(Float)
    pre_settlement_price = Column(Float)

    open_date = Column(String)

    trade_id = Column(UINT64)
    trade_time = Column(Integer)

    margin_ratio = Column(Float)
    contract_multiplier = Column(Integer)

class Location(Base):
    __tablename__ = "location"
    uid = Column(Integer, primary_key = True)
    info = Column(Json)

class AlgoOrder(Base):
    __tablename__ = "algo_order"
    order_id = Column(UINT64, primary_key=True)
    algo_type = Column(String)
    sender_uid = Column(Integer)
    update_time = Column(Integer)
    params = Column(Json)
    status = Column(Json)
    active = Column(Boolean)