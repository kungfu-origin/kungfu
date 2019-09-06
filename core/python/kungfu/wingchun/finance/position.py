
from pyyjj import hash_str_32
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import *
import datetime

def get_uname(instrument_id, exchange_id, direction):
    return "{}.{}.{}".format(instrument_id, exchange_id, int(direction))

def get_uid(instrument_id, exchange_id, direction):
    uname = get_uname(instrument_id, exchange_id, direction)
    return hash_str_32(uname)

class Position:
    registry = {}
    def __init__(self, **kwargs):
        self._instrument_id = kwargs.pop("instrument_id")
        self._exchange_id = kwargs.pop("exchange_id")
        self._direction = kwargs.pop("direction", Direction.Long)
        self._uname = get_uname(self._instrument_id, self._exchange_id, self._direction)
        self._uid = get_uid(self._instrument_id, self._exchange_id, self._direction)
        self._trading_day = kwargs.pop("trading_day")
        if isinstance(self._trading_day, str):
            self._trading_day = datetime.datetime.strptime(self._trading_day, "%Y%m%d")
        self._ledger = kwargs.pop("ledger", None)

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry[cls._INSTRUMENT_TYPE] = cls

    def __repr__(self):
        return "%s(%r)" % (self.__class__, self.message["data"])

    @classmethod
    def factory(cls, instrument_type, **kwargs):
        return cls.registry[instrument_type](**kwargs)

    @property
    def trading_day(self):
        return self._trading_day

    @trading_day.setter
    def trading_day(self, value):
        self._trading_day = value

    @property
    def instrument_id(self):
        return self._instrument_id

    @property
    def exchange_id(self):
        return self._exchange_id

    @property
    def direction(self):
        return self._direction

    @property
    def uid(self):
        return self._uid

    @property
    def uname(self):
        return self._uname

    @property
    def margin(self):
        raise NotImplementationError

    @property
    def market_value(self):
        raise NotImplementationError

    @property
    def last_price(self):
        raise NotImplementationError

    @property
    def realized_pnl(self):
        raise NotImplementationError

    @property
    def unrealized_pnl(self):
        raise NotImplementationError

    @property
    def ledger(self):
        return self._ledger

    @ledger.setter
    def ledger(self, value):
        self._ledger = value

    def apply_trade(self, trade):
        raise NotImplementationError

    def apply_quote(self, quote):
        raise NotImplementationError

    def switch_day(self, trading_day):
        raise NotImplementationError

class StockPosition(Position):
    _INSTRUMENT_TYPE = InstrumentType.Stock
    def __init__(self, **kwargs):
        super(StockPosition, self).__init__(**kwargs)
        self._last_price = kwargs.pop("last_price", 0.0)
        self._volume = kwargs.pop("volume", 0)
        self._yesterday_volume = kwargs.pop("yesterday_volume", 0)
        self._avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self._position_cost_price = kwargs.pop("position_cost_price", 0.0)
        if not is_valid_price(self._avg_open_price) and is_valid_price(self._position_cost_price):
            self._avg_open_price = self._position_cost_price
        elif not is_valid_price(self._position_cost_price) and is_valid_price(self._avg_open_price):
            self._position_cost_price = self._avg_open_price
        self._pre_close_price = kwargs.pop("pre_close_price", 0.0)
        self._close_price = kwargs.pop("close_price", 0.0)
        self._realized_pnl = kwargs.pop("realized_pnl", 0.0)

    @property
    def message(self):
        return {
            "msg_type": int(MsgType.Position),
            "data":  {          
                "instrument_id": self.instrument_id,
                "exchange_id":self.exchange_id,
                "instrument_type": int(InstrumentType.Stock),
                "direction": int(self.direction),
                "trading_day": self.trading_day.strftime("%Y%m%d"),
                "volume":self.volume,
                "yesterday_volume": self.yesterday_volume,
                "last_price": self.last_price,
                "avg_open_price": self.avg_open_price,
                "position_cost_price": self._position_cost_price,
                "close_price": self.close_price,
                "pre_close_price": self.pre_close_price,
                "realized_pnl": self.realized_pnl,
                "unrealized_pnl": self.unrealized_pnl
            }
        }

    @property
    def volume(self):
        return self._volume

    @property
    def yesterday_volume(self):
        return self._yesterday_volume

    @property
    def close_price(self):
        return self._close_price

    @property
    def pre_close_price(self):
        return self._pre_close_price

    @property
    def last_price(self):
        return self._last_price

    @property
    def margin(self):
        return 0.0

    @property
    def market_value(self):
        return self._volume * (self._last_price if self._last_price > 0.0 else self._avg_open_price)

    @property
    def avg_open_price(self):
        return self._avg_open_price

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @property
    def unrealized_pnl(self):
        return (self._last_price - self._avg_open_price) * self._volume if self._last_price > 0.0 else 0.0

    def __repr__(self):
        return "%s(%r)" % (self.__class__, self.message["data"])

    def merge(self, position):
        self.ledger._ctx.logger.info("merge {} with {}".format(self, position))
        if self.realized_pnl == 0.0 and position.realized_pnl != 0.0:
            self.realized_pnl = position.realized_pnl
        self._volume = position.volume
        self._yesterday_volume = position.yesterday_volume
        self.ledger._ctx.logger.info("merged {}".format(self))
        return self

    def apply_trade(self, trade):        
        if trade.side == Side.Buy:
            self._apply_buy(trade.price, trade.volume)
        else:
            self._apply_sell(trade.price, trade.volume)
        self.ledger.dispatch([self.ledger.message, self.message])

    def apply_quote(self, quote):
        self.ledger._ctx.logger.debug("{} apply quote[(inst){}(last_price){}(pre_close_price){}(close_price){}]".format(self._uname, quote.instrument_id, quote.last_price, quote.pre_close_price, quote.close_price))
        if is_valid_price(quote.close_price):
            self._apply_settlement(quote.close_price)
        elif is_valid_price(quote.last_price):
            self._last_price = quote.last_price
        self.ledger.dispatch([self.ledger.message, self.message])

    def switch_day(self, trading_day):
        if not is_valid_price(self.close_price):
            self._apply_settlement(self.last_price)
        self._pre_close_price = self.close_price
        self._yesterday_volume = self.volume
        self._close_price = 0.0
        self.trading_day = trading_day
        self.ledger.dispatch([self.ledger.message])

    def _apply_settlement(self, close_price):
        self._last_price = close_price
        self._close_price = close_price

    def _apply_sell(self, price, volume):
        realized_pnl = self._calculate_realized_pnl(price, volume)
        self._yesterday_volume -= volume
        self._volume -= volume
        self._realized_pnl += realized_pnl
        self.ledger.realized_pnl += realized_pnl
        self.ledger.avail += (realized_pnl + price * volume)

    def _apply_buy(self, price, volume):
        self._avg_open_price = (self._avg_open_price * self._volume + price * volume) / (self._volume + volume)
        self._volume += volume
        self.ledger.avail -= price * volume

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self._avg_open_price) * trade_volume

class FuturePositionDetail:
    def __init__(self, **kwargs):
        self._instrument_id = kwargs.pop("instrument_id")
        self._exchange_id = kwargs.pop("exchange_id")
        self._open_price = kwargs.pop("open_price")
        self._open_date = kwargs.pop("open_date")
        if isinstance(self._open_date, str):
            self._open_date = datetime.datetime.strptime(self._open_date, "%Y%m%d")
        self._trading_day = kwargs.pop("trading_day")
        if isinstance(self._trading_day, str):
            self._trading_day = datetime.datetime.strptime(self._trading_day, "%Y%m%d")
        self._direction = kwargs.pop("direction")
        self._volume = kwargs.pop("volume")
        self._contract_multiplier = kwargs.pop("contract_multiplier")
        self._margin_ratio = kwargs.pop("margin_ratio", None)
        if not self._margin_ratio:
            self._margin_ratio = kwargs.pop("long_margin_ratio") if self._direction == Direction.Long else kwargs.pop("short_margin_ratio")
        self._settlement_price = kwargs.pop("settlement_price", 0.0)
        self._pre_settlement_price = kwargs.pop("pre_settlement_price", 0.0)
        self._last_price = kwargs.pop("last_price", 0.0)

        self._trade_id = kwargs.pop("trade_id")
        if isinstance(self._trade_id, str):
            self._trade_id = int(self._trade_id)
        self._trade_time = kwargs.pop("trade_time")

    @property
    def message(self):
        return {
            "msg_type": int(MsgType.PositionDetail),
            "data": {
                "trading_day": self._trading_day.strftime("%Y%m%d"),
                "instrument_id": self._instrument_id,
                "exchange_id": self._exchange_id,              
                "direction": int(self.direction),
                "volume":self.volume,
                "margin": self.margin,
                "last_price": self.last_price,
                "open_price": self.open_price,
                "open_date": self.open_date.strftime("%Y%m%d"),
                "settlement_price": self.settlement_price,
                "pre_settlement_price": self.pre_settlement_price,
                "unrealized_pnl": self.unrealized_pnl,
                "position_pnl": self.position_pnl,
                "margin_ratio": self._margin_ratio,
                "contract_multiplier": self._contract_multiplier,
                "trade_id": self._trade_id,
                "trade_time": self._trade_time
            }
        }

    @property
    def direction(self):
        return self._direction            

    @property
    def open_date(self):
        return self._open_date

    @property
    def open_price(self):
        return self._open_price

    @property
    def last_price(self):
        return self._last_price

    @property
    def market_value(self):
        return self._select_last_price() * self.volume * self._contract_multiplier

    @property
    def trading_day(self):
        return self._trading_day

    @property
    def margin(self):
        price = self._select_price_for_margin()
        return self._calculate_margin(price, self.volume)

    @property
    def volume(self):
        return self._volume

    @property
    def pre_settlement_price(self):
        return self._pre_settlement_price

    @property
    def settlement_price(self):
        return self._settlement_price

    @property
    def unrealized_pnl(self):
        last_price = self._select_last_price()
        return (last_price - self._open_price) * self._volume * self._contract_multiplier * (1 if self._direction == int(Direction.Long) else -1)

    @property
    def position_pnl(self):
        last_price = self._select_last_price()
        cost_price = self._select_cost_price()
        return  (last_price - cost_price) * self._volume * self._contract_multiplier * (1 if self._direction == int(Direction.Long) else -1)

    def __repr__(self):
        return "%s(%r)" % (self.__class__, self.message["data"])

    def apply_close(self, price, volume):
        volume_closed = min(self._volume, volume)
        realized_pnl = self._calculate_realized_pnl(price, volume_closed)
        margin_delta = self._calculate_margin(price, volume_closed) * -1
        self._volume = self._volume - volume_closed

        return (volume_closed, realized_pnl, margin_delta)

    def apply_settlement(self, settlement_price):
        pre_margin = self.margin
        self._settlement_price = settlement_price
        return pre_margin - self.margin

    def switch_day(self, trading_day):
        self._trading_day = trading_day
        self._pre_settlement_price = self._settlement_price
        self._settlement_price = 0.0

    def update_last_price(self, last_price):
        pre_position_pnl = self.position_pnl
        self._last_price = last_price
        return pre_position_pnl - self.position_pnl

    def _select_last_price(self):
        if self._settlement_price > 0.0:
            return self._settlement_price
        elif self._last_price > 0.0:
            return self._last_price
        elif self.trading_day != self._open_date and self._pre_settlement_price > 0.0:
            return self._pre_settlement_price
        else:
            return self._open_price

    def _select_cost_price(self):
        if self.trading_day != self.open_date and self._pre_settlement_price > 0.0:
            return self._pre_settlement_price
        else:
            return self.open_price

    def _select_price_for_margin(self):
        price = self._settlement_price if self._trading_day == self._open_date else self._pre_settlement_price
        if price <= 0.0:
            price = self._open_price
        return price

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self._open_price) * trade_volume * self._contract_multiplier * (1 if self._direction == int(Direction.Long) else -1)

    def _calculate_margin(self, price, volume):
        return self._contract_multiplier * price * volume * self._margin_ratio

class FuturePosition(Position):
    _INSTRUMENT_TYPE = InstrumentType.Future
    def __init__(self, **kwargs):
        super(FuturePosition, self).__init__(**kwargs)
        details = kwargs.pop("details", [])
        self._details = [detail for detail in details if detail.direction == self.direction]
        self._contract_multiplier = kwargs.pop("contract_multiplier")
        if "margin_ratio" in kwargs:
            self._margin_ratio = kwargs.pop("margin_ratio")
        elif self.direction == Direction.Long:
            self._margin_ratio = kwargs.pop("long_margin_ratio")
        elif self.direction == Direction.Short:
            self._margin_ratio = kwargs.pop("short_margin_ratio")

        self._pre_settlement_price = kwargs.pop("pre_settlement_price", 0.0)            
        self._settlement_price = kwargs.pop("settlement_price", 0.0)            
        self._last_price = kwargs.pop("last_price", 0.0)

        if "realized_pnl" in kwargs:
            self._realized_pnl = kwargs.pop("realized_pnl")
        elif self.direction == Direction.Long:
            self._realized_pnl = kwargs.pop("long_realized_pnl", 0.0)
        elif self.direction == Direction.Short:
            self._realized_pnl = kwargs.pop("short_realized_pnl", 0.0)

    @property
    def message(self):
        return {
            "msg_type": int(MsgType.Position),
            "data": {
                "trading_day": self._trading_day.strftime("%Y%m%d"),
                "instrument_id": self._instrument_id,
                "exchange_id": self._exchange_id,
                "instrument_type": int(InstrumentType.Future),
                "direction": int(self.direction),
                "volume": self.volume,
                "yesterday_volume": self.yesterday_volume,
                "last_price": self.last_price,
                "settlement_price": self._settlement_price,
                "pre_settlement_price": self._pre_settlement_price,
                "unrealized_pnl": self.unrealized_pnl,
                "position_pnl": self.position_pnl,
                "realized_pnl": self.realized_pnl,
                "margin": self.margin,
                "margin_ratio": self._margin_ratio,
                "contract_multiplier": self._contract_multiplier
            }
        }

    @property
    def last_price(self):
        return self._last_price

    @property
    def details(self):
        return self._details

    @property
    def volume(self):
        return sum([detail.volume for detail in self._details])

    @property
    def yesterday_volume(self):
        return sum([detail.volume for detail in self._details if detail.open_date != detail.trading_day])

    @property
    def unrealized_pnl(self):
        return sum([detail.unrealized_pnl for detail in self._details])

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @property
    def margin(self):
        return sum([detail.margin for detail in self._details])

    @property
    def position_pnl(self):
         return sum([detail.position_pnl for detail in self._details])

    def merge(self, position):
        self.ledger._ctx.logger.info("merge {} with {}".format(self, position))
        if self.realized_pnl == 0.0 and position.realized_pnl != 0.0:
            self.realized_pnl = position.realized_pnl
        self._details = position.details
        self.ledger._ctx.logger.info("merged {}".format(self))
        return self

    def apply_trade(self, trade):
        if trade.offset == Offset.Open:
            self._apply_open(trade)
        elif trade.offset == Offset.Close or trade.offset == Offset.CloseToday:            
            self._apply_close(trade)

    def apply_quote(self, quote):
        if is_valid_price(quote.settlement_price):
            self._apply_settlement(quote.settlement_price)
        elif is_valid_price(quote.last_price):
            self._update_last_price(quote.last_price)

    def switch_day(self, trading_day):
        if not is_valid_price(self._settlement_price):
            self._apply_settlement(self._last_price)
        for detail in self.details:
            detail.switch_day(trading_day)
        self._pre_settlement_price = self._settlement_price
        self._settlement_price = 0.0
        self.ledger.dispatch(self.message)

    def _apply_close(self, trade):
        self.ledger._ctx.logger.debug("{} close {}, volume {} price {}".format(self.instrument_id, "long" if trade.side == Side.Sell else "short", trade.volume, trade.price))
        volume_left = trade.volume
        messages = []
        while volume_left > 0 and len(self._details) > 0:
            detail = self._details[0]
            volume_closed, close_pnl, margin_delta = detail.apply_close(trade.price, volume_left)
            self.ledger._ctx.logger.debug("detail({},{}|{}) {} closed {} left {}, pnl {}, margin delta {}".format(detail._trade_id, detail.open_date, detail.open_price, detail.volume, volume_closed, detail.volume, close_pnl, margin_delta))
            self._realized_pnl += close_pnl
            self.ledger.avail += (close_pnl - margin_delta)
            volume_left -= volume_closed
            messages.append(detail.message)
            if detail.volume <= 0:
                self._details.pop(0)
        messages.append(self.ledger.message)
        messages.append(self.message)
        self.ledger.dispatch(messages)

    def _apply_open(self, trade):
        self.ledger._ctx.logger.debug("{} open volume: {} price: {}".format(self._uname, trade.volume, trade.price))
        detail = FuturePositionDetail(instrument_id = self.instrument_id,
                                      exchange_id = self.exchange_id,
                                      trading_day = self.ledger.trading_day,
                                      open_date = self.ledger.trading_day,
                                      open_price = trade.price,
                                      volume = trade.volume,
                                      direction = self.direction,
                                      margin_ratio = self._margin_ratio,
                                      contract_multiplier = self._contract_multiplier,
                                      pre_settlement_price = self._pre_settlement_price,
                                      last_price = self._last_price,
                                      settlement_price = self._settlement_price,
                                      trade_id = trade.trade_id,
                                      trade_time=trade.trade_time
                                      )
        self.ledger.avail -= detail.margin
        self.ledger._ctx.logger.debug("after open, margin delta {}".format(detail.margin))
        self._details.append(detail)
        self.ledger.dispatch([detail.message, self.ledger.message, self.message])

    def _apply_settlement(self, settlement_price):
        self.ledger._ctx.logger.debug("{} apply settlement with price {}".format(self._uname, settlement_price))
        self._settlement_price = settlement_price
        margin_delta = sum([detail.apply_settlement(settlement_price) for detail in self.details])
        self.ledger._ctx.logger.debug("margin delta: {}".format(margin_delta))
        self.ledger.avail -= margin_delta
        self.ledger.dispatch([self.ledger.message, self.message])

    def _update_last_price(self, last_price):
        self.ledger._ctx.logger.debug("update {} last price from {} to {}".format(self.instrument_id, self.last_price, last_price))
        self._last_price = last_price
        position_pnl_delta = sum([detail.update_last_price(last_price) for detail in self.details])
        self.ledger._ctx.logger.debug("position pnl delta: {}".format(position_pnl_delta))
        self.ledger.dispatch([self.ledger.message, self.message])
