from pyyjj import hash_str_32
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import *
import datetime
import kungfu.wingchun.msg as wc_msg

DATE_FORMAT = "%Y%m%d"

def get_uname(instrument_id, exchange_id, direction):
    return "{}.{}.{}".format(instrument_id, exchange_id, int(direction))

def get_uid(instrument_id, exchange_id, direction):
    uname = get_uname(instrument_id, exchange_id, direction)
    return hash_str_32(uname)

class Position:
    registry = {}
    def __init__(self, ctx, book, **kwargs):
        self.ctx = ctx
        self.book = book
        self.instrument_id = kwargs["instrument_id"]
        self.exchange_id = kwargs["exchange_id"]
        self.direction = kwargs.get("direction", Direction.Long)
        if isinstance(self.direction, int):
            self.direction = Direction(self.direction)
        self.uname = get_uname(self.instrument_id, self.exchange_id, self.direction)
        self.uid = get_uid(self.instrument_id, self.exchange_id, self.direction)
        self.trading_day = kwargs.pop("trading_day", None)
        if not self.trading_day:
            self.trading_day = self.ctx.trading_day
        elif isinstance(self.trading_day, str):
            self.trading_day = datetime.datetime.strptime(self.trading_day, DATE_FORMAT)
        self._last_check = 0

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        for inst_type in cls._INSTRUMENT_TYPES:
            cls.registry[inst_type] = cls

    def __repr__(self):
        return self.event.data.__repr__()

    def _on_interval_check(self, now):
        if self._last_check + int(1e9) * 30 < now:
            self.subject.on_next(self.event)
            self._last_check = now

    @classmethod
    def factory(cls, ctx, book, **kwargs):
        instrument_type = get_instrument_type(kwargs["instrument_id"], kwargs["exchange_id"])
        return cls.registry[instrument_type](ctx, book, **kwargs)

    @property
    def instrument_type(self):
        return get_instrument_type(self.instrument_id, self.exchange_id)

    @property
    def subject(self):
        return self.book.subject

    def apply_trade(self, trade):
        raise NotImplementedError

    def apply_quote(self, quote):
        raise NotImplementedError

    def apply_trading_day(self, trading_day):
        raise NotImplementedError

class StockPosition(Position):
    _INSTRUMENT_TYPES = InstrumentTypeInStockAccount

    def __init__(self, ctx, book, **kwargs):
        super(StockPosition, self).__init__(ctx, book, **kwargs)
        self.last_price = kwargs.pop("last_price", 0.0)
        self.volume = kwargs.pop("volume", 0)
        self.yesterday_volume = kwargs.pop("yesterday_volume", 0)
        self.avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self.position_cost_price = kwargs.pop("position_cost_price", 0.0)
        if not is_valid_price(self.avg_open_price) and is_valid_price(self.position_cost_price):
            self.avg_open_price = self.position_cost_price
        elif not is_valid_price(self.position_cost_price) and is_valid_price(self.avg_open_price):
            self.position_cost_price = self.avg_open_price
        self.pre_close_price = kwargs.pop("pre_close_price", 0.0)
        self.close_price = kwargs.pop("close_price", 0.0)
        self.realized_pnl = kwargs.pop("realized_pnl", 0.0)

        if ctx.trading_day:
            self.apply_trading_day(ctx.trading_day)

    @property
    def event(self):
        data = pywingchun.Position()
        data.instrument_id = self.instrument_id
        data.exchange_id = self.exchange_id
        data.last_price = self.last_price
        data.pre_close_price = self.pre_close_price
        data.close_price = self.close_price
        data.avg_open_price = self.avg_open_price
        data.position_cost_price = self.position_cost_price
        data.realized_pnl = self.realized_pnl
        data.unrealized_pnl = self.unrealized_pnl
        data.volume = self.volume
        data.yesterday_volume = self.yesterday_volume
        data.direction = self.direction
        return self.book.make_event(wc_msg.Position, data)

    @property
    def market_value(self):
        return self.volume * (self.last_price if self.last_price > 0.0 else self.avg_open_price)

    @property
    def unrealized_pnl(self):
        return (self.last_price - self.avg_open_price) * self.volume if self.last_price > 0.0 else 0.0

    @property
    def margin(self):
        return 0.0

    def apply_trade(self, trade):
        if trade.side == Side.Buy:
            self._apply_buy(trade.price, trade.volume)
        elif trade.side == Side.Sell:
            self._apply_sell(trade.price, trade.volume)
        else:
            pass
        self.book.subject.on_next(self.event)

    def apply_quote(self, quote):
        if is_valid_price(quote.close_price):
            self.close_price = quote.close_price
        if is_valid_price(quote.last_price):
            self.last_price = quote.last_price
        if is_valid_price(quote.pre_close_price):
            self.pre_close_price = quote.pre_close_price
        self._on_interval_check(self.ctx.now())

    def apply_trading_day(self, trading_day):
        if not self.trading_day == trading_day:
            self.ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            if is_valid_price(self.close_price):
                self.pre_close_price = self.close_price
            elif is_valid_price(self.last_price):
                self.pre_close_price = self.last_price
            self.yesterday_volume = self.volume
            self.close_price = 0.0
            self.trading_day = trading_day
            self.book.subject.on_next(self.event)

    def _apply_sell(self, price, volume):
        if self.yesterday_volume < volume:
            raise Exception("{} current yesterday volume is {}, {} to sell".format(self.uname, self.yesterday_volume, volume))
        realized_pnl = self._calculate_realized_pnl(price, volume)
        self.yesterday_volume -= volume
        self.volume -= volume
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl
        self.book.avail += (realized_pnl + price * volume)

    def _apply_buy(self, price, volume):
        self.avg_open_price = (self.avg_open_price * self.volume + price * volume) / (self.volume + volume)
        self.volume += volume
        self.book.avail -= price * volume

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self.avg_open_price) * trade_volume

class FuturePosition(Position):
    _INSTRUMENT_TYPES = [InstrumentType.Future]
    def __init__(self, ctx, book, **kwargs):
        super(FuturePosition, self).__init__(ctx, book, **kwargs)
        inst_info = self.ctx.get_inst_info(self.instrument_id)
        self.contract_multiplier = inst_info["contract_multiplier"]
        self.margin_ratio = inst_info["long_margin_ratio"] if self.direction == Direction.Long else inst_info["short_margin_ratio"]
        self.pre_settlement_price = kwargs.pop("pre_settlement_price", 0.0)
        self.settlement_price = kwargs.pop("settlement_price", 0.0)
        self.last_price = kwargs.pop("last_price", 0.0)
        self.avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self.position_cost_price = kwargs.pop("position_cost_price", 0.0)
        self.volume = kwargs.pop("volume", 0)
        self.yesterday_volume = kwargs.pop("yesterday_volume", 0)
        self.margin = kwargs.pop("margin", 0.0)
        self.realized_pnl = kwargs.pop("realized_pnl", 0.0)

        if ctx.trading_day:
            self.apply_trading_day(ctx.trading_day)

    @property
    def unrealized_pnl(self):
        if not is_valid_price(self.last_price):
            return 0.0
        else:
            return (self.last_price - self.avg_open_price) * self.volume * \
                   self.contract_multiplier * (1 if self.direction == Direction.Long else -1)
    @property
    def position_pnl(self):
        if is_valid_price(self.last_price):
            return self.contract_multiplier * self.last_price * self.volume * self.margin_ratio - self.margin
        else:
            return 0.0

    @property
    def event(self):
        data = pywingchun.Position()
        data.instrument_id = self.instrument_id
        data.exchange_id = self.exchange_id
        data.instrument_type = self.instrument_type
        data.last_price = self.last_price
        data.pre_settlement_price = self.pre_settlement_price
        data.settlement_price = self.settlement_price
        data.avg_open_price = self.avg_open_price
        data.position_cost_price = self.position_cost_price
        data.realized_pnl = self.realized_pnl
        data.unrealized_pnl = self.unrealized_pnl
        data.position_pnl = self.position_pnl
        data.volume = self.volume
        data.yesterday_volume = self.yesterday_volume
        data.direction = self.direction
        data.margin = self.margin
        return self.book.make_event(wc_msg.Position, data)

    def apply_trade(self, trade):
        if trade.offset == Offset.Open:
            self._apply_open(trade)
        elif trade.offset == Offset.Close or trade.offset == Offset.CloseToday or trade.offset == Offset.CloseYesterday:
            self._apply_close(trade)
        self.book.subject.on_next(self.event)

    def apply_quote(self, quote):
        if is_valid_price(quote.settlement_price):
            self.settlement_price = quote.settlement_price
            pre_margin = self.margin
            self.margin = self.contract_multiplier * self.settlement_price * self.volume * self.margin_ratio
            self.book.avail -= (self.margin - pre_margin)
        if is_valid_price(quote.last_price):
            self.last_price = quote.last_price
        if is_valid_price(quote.pre_settlement_price):
            self.pre_settlement_price = quote.pre_settlement_price
        self._on_interval_check(self.ctx.now())

    def apply_trading_day(self, trading_day):
        if not self.trading_day == trading_day:
            self.ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            if not is_valid_price(self.settlement_price):
                if is_valid_price(self.last_price):
                    self.ctx.logger.warn("{} no valid settlement price found, use last price {} instead".format(self.uname, self.last_price))
                    self.settlement_price = self.last_price
                else:
                    self.ctx.logger.warn("{} no valid settlement price found, use avg open price {} instead".format(self.uname, self.avg_open_price))
                    self.settlement_price = self.avg_open_price
            pre_margin = self.margin
            self.margin = self.contract_multiplier * self.settlement_price * self.volume * self.margin_ratio
            self.book.avail -= (self.margin - pre_margin)
            self.pre_settlement_price = self.settlement_price
            self.last_price = self.settlement_price
            self.settlement_price = 0.0
            self.yesterday_volume = self.volume
            self.book.subject.on_next(self.event)
            self.trading_day = trading_day

    def _apply_close(self, trade):
        if self.volume < trade.volume:
            raise Exception("{} over close position, current volume is {}, {} to close".format(self.uname, self.volume, trade.volume))
        if trade.offset == Offset.CloseToday and self.volume - self.yesterday_volume < trade.volume:
            raise Exception("{} over close today position, current volume is {}, {} to close".format(self.uname, self.volume - self.yesterday_volume, trade.volume))
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        self.margin -= margin
        self.book.avail += margin
        self.volume -= trade.volume
        if self.yesterday_volume > 0 and trade.offset != Offset.CloseToday:
            self.yesterday_volume = 0 if self.yesterday_volume <= trade.volume else \
                self.yesterday_volume - trade.volume
        realized_pnl = (trade.price - self.avg_open_price) * trade.volume * \
                       self.contract_multiplier * (1 if self.direction == Direction.Long else -1)
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl

    def _apply_open(self, trade):
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        self.margin += margin
        self.book.avail -= margin
        self.avg_open_price = (self.avg_open_price * self.volume + trade.volume * trade.price) / (self.volume + trade.volume)
        self.volume += trade.volume

# 暂时抄一份期货的，等2.2重写，抄一份是为了以防后续还要改的时候不改期货的
class StockOptionPosition(Position):
    _INSTRUMENT_TYPES = [InstrumentType.StockOption]
    def __init__(self, ctx, book, **kwargs):
        super(StockOptionPosition, self).__init__(ctx, book, **kwargs)
        self.pre_settlement_price = kwargs.pop("pre_settlement_price", 0.0)
        self.settlement_price = kwargs.pop("settlement_price", 0.0)
        self.last_price = kwargs.pop("last_price", 0.0)
        self.avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self.position_cost_price = kwargs.pop("position_cost_price", 0.0)
        self.volume = kwargs.pop("volume", 0)
        self.yesterday_volume = kwargs.pop("yesterday_volume", 0)
        self.margin = kwargs.pop("margin", 0.0)
        self.realized_pnl = kwargs.pop("realized_pnl", 0.0)

        if ctx.trading_day:
            self.apply_trading_day(ctx.trading_day)

    @property
    def unrealized_pnl(self):
        if not is_valid_price(self.last_price):
            return 0.0
        else:
            return (self.last_price - self.avg_open_price) * self.volume * \
                   10000 * (1 if self.direction == Direction.Long else -1)
    @property
    def position_pnl(self):
        if is_valid_price(self.last_price):
            return 10000 * self.last_price * self.volume  - self.margin
        else:
            return 0.0

    @property
    def event(self):
        data = pywingchun.Position()
        data.instrument_id = self.instrument_id
        data.exchange_id = self.exchange_id
        data.instrument_type = self.instrument_type
        data.last_price = self.last_price
        data.pre_settlement_price = self.pre_settlement_price
        data.settlement_price = self.settlement_price
        data.avg_open_price = self.avg_open_price
        data.position_cost_price = self.position_cost_price
        data.realized_pnl = self.realized_pnl
        data.unrealized_pnl = self.unrealized_pnl
        data.position_pnl = self.position_pnl
        data.volume = self.volume
        data.yesterday_volume = self.yesterday_volume
        data.direction = self.direction
        data.margin = self.margin
        return self.book.make_event(wc_msg.Position, data)

    def apply_trade(self, trade):
        # 只处理期权普通买卖，不处理锁仓行权等
        if trade.side == Side.Buy or trade.side == Side.Sell:
            if trade.offset == Offset.Open:
                self._apply_open(trade)
            elif trade.offset == Offset.Close or trade.offset == Offset.CloseToday or trade.offset == Offset.CloseYesterday:
                self._apply_close(trade)
        else:
            # 暂不处理锁仓行权
            pass
        self.book.subject.on_next(self.event)

    def apply_quote(self, quote):
        if is_valid_price(quote.settlement_price):
            self.settlement_price = quote.settlement_price
            pre_margin = self.margin
            self.margin = 10000 * self.settlement_price * self.volume
            self.book.avail -= (self.margin - pre_margin)
        if is_valid_price(quote.last_price):
            self.last_price = quote.last_price
        if is_valid_price(quote.pre_settlement_price):
            self.pre_settlement_price = quote.pre_settlement_price
        self._on_interval_check(self.ctx.now())

    def apply_trading_day(self, trading_day):
        if not self.trading_day == trading_day:
            self.ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            if not is_valid_price(self.settlement_price):
                if is_valid_price(self.last_price):
                    self.ctx.logger.warn("{} no valid settlement price found, use last price {} instead".format(self.uname, self.last_price))
                    self.settlement_price = self.last_price
                else:
                    self.ctx.logger.warn("{} no valid settlement price found, use avg open price {} instead".format(self.uname, self.avg_open_price))
                    self.settlement_price = self.avg_open_price
            pre_margin = self.margin
            self.margin = 10000 * self.settlement_price * self.volume
            self.book.avail -= (self.margin - pre_margin)
            self.pre_settlement_price = self.settlement_price
            self.last_price = self.settlement_price
            self.settlement_price = 0.0
            self.yesterday_volume = self.volume
            self.book.subject.on_next(self.event)
            self.trading_day = trading_day

    def _apply_close(self, trade):
        if self.volume < trade.volume:
            raise Exception("{} over close position, current volume is {}, {} to close".format(self.uname, self.volume, trade.volume))
        if trade.offset == Offset.CloseToday and self.volume - self.yesterday_volume < trade.volume:
            raise Exception("{} over close today position, current volume is {}, {} to close".format(self.uname, self.volume - self.yesterday_volume, trade.volume))
        amount = trade.price * trade.volume * 10000
        if trade.side == Side.Buy:
            self.margin -= amount
        self.book.avail += amount
        self.volume -= trade.volume
        if self.yesterday_volume > 0 and trade.offset != Offset.CloseToday:
            self.yesterday_volume = 0 if self.yesterday_volume <= trade.volume else \
                self.yesterday_volume - trade.volume
        realized_pnl = (trade.price - self.avg_open_price) * trade.volume * \
                       10000 * (1 if self.direction == Direction.Long else -1)
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl

    def _apply_open(self, trade):
        amount = trade.price * trade.volume * 10000;
        if trade.side == Side.Sell:
            self.margin += amount
        self.book.avail -= amount
        self.avg_open_price = (self.avg_open_price * self.volume + trade.volume * trade.price) / (self.volume + trade.volume)
        self.volume += trade.volume

