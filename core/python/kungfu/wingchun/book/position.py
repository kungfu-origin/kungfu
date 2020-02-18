from pykungfu.yijinjing import hash_str_32
from kungfu.wingchun.constants import *
from kungfu.wingchun.utils import *
import datetime
import kungfu.wingchun.msg as wc_msg
import pykungfu

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

    def apply_order_input(self, input):
        raise NotImplementedError

    def apply_order(self, order):
        raise NotImplementedError

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
        self.frozen_total = kwargs.pop("frozen_total", 0)
        self.frozen_yesterday = kwargs.pop("frozen_yesterday", 0)
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

        self.min_commission = kwargs.pop("min_commission", 5)
        self.commission_ratio = kwargs.pop("commission_ratio", 0.0008)
        self._commission_map = {}

        if ctx.trading_day:
            self.apply_trading_day(ctx.trading_day)

    @property
    def event(self):
        data = pykungfu.longfist.types.Position()
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
        data.frozen_total = self.frozen_total
        data.frozen_yesterday = self.frozen_yesterday
        data.direction = self.direction
        return self.book.make_event(wc_msg.Position, data)

    @property
    def market_value(self):
        return self.volume * (self.last_price if self.last_price > 0.0 else self.avg_open_price)

    @property
    def unrealized_pnl(self):
        return (self.last_price - self.avg_open_price) * self.volume if self.last_price > 0.0 else 0.0

    @property
    def sellable(self):
        return self.yesterday_volume - self.frozen_yesterday

    @property
    def margin(self):
        return 0.0

    def apply_order_input(self, input):
        if input.side == Side.Sell:
            if self.sellable < input.volume:
                raise ValueError("{} to sell, but {} sellable for {}.{}".format(input.volume, self.sellable, self.instrument_id, self.exchange_id))
            self.frozen_total += input.volume
            self.frozen_yesterday += input.volume
        elif input.side == Side.Buy:
            self.book.frozen_cash += input.volume * input.frozen_price
            self.book.avail -= input.volume * input.frozen_price
        self.book.subject.on_next(self.event)

    def apply_order(self, order):
        if not order.status in [pykungfu.longfist.enum.OrderStatus.Submitted, pykungfu.longfist.enum.OrderStatus.Pending, pykungfu.longfist.enum.OrderStatus.PartialFilledActive] and order.volume_left > 0:
            if order.side == Side.Sell:
                self.frozen_total -= order.volume_left
                self.frozen_yesterday -= order.volume_left
                self.book.subject.on_next(self.event)
            elif order.side == Side.Buy:
                frozen_amt = order.volume_left * order.frozen_price
                self.book.frozen_cash -= frozen_amt
                self.book.avail += frozen_amt
                self.book.subject.on_next(self.book.event)

    def apply_trade(self, trade):
        if trade.side == Side.Buy:
            self._apply_buy(trade)
        elif trade.side == Side.Sell:
            self._apply_sell(trade)
        else:
            # ignore stock options lock/unlock/exec/drop operations
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

    def _apply_sell(self, trade):
        if self.yesterday_volume < trade.volume:
            raise ValueError("{} current yesterday volume is {}, {} to sell".format(self.uname, self.yesterday_volume, trade.volume))
        if self.frozen_total < trade.volume:
            raise ValueError("{} current frozen volume is {}, {} to sell".format(self.uname, self.frozen_total, trade.volume))
        realized_pnl = self._calculate_realized_pnl(trade.price, trade.volume)
        commission = self._calculate_commission(trade)
        tax = self._calculate_tax(trade)
        trade.commission = commission
        trade.tax = tax
        self.frozen_total -= trade.volume
        self.frozen_yesterday -= trade.volume
        self.yesterday_volume -= trade.volume
        self.volume -= trade.volume
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl
        self.book.avail += trade.price * trade.volume
        self.book.avail -= commission
        self.book.avail -= tax
        self.book.intraday_fee += (commission + tax)
        self.book.accumulated_fee += (commission + tax)

    def _apply_buy(self, trade):
        self.avg_open_price = (self.avg_open_price * self.volume + trade.price * trade.volume) / (self.volume + trade.volume)
        commission = self._calculate_commission(trade)
        tax = self._calculate_tax(trade)
        trade.commission = commission
        trade.tax = tax
        self.volume += trade.volume
        self.book.frozen_cash -= self.book.get_frozen_price(trade.order_id) * trade.volume
        self.book.avail -= commission
        self.book.avail -= tax
        self.book.intraday_fee += (commission + tax)
        self.book.accumulated_fee += (commission + tax)

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self.avg_open_price) * trade_volume

    def _calculate_tax(self, trade):
        return trade.price * trade.volume * 0.001 if trade.side == Side.Sell else 0.0

    def _calculate_commission(self, trade):
        order_id = trade.order_id
        if order_id not in self._commission_map:
            self._commission_map[order_id] = self.min_commission
        commission = self._commission_map[order_id]
        amt = trade.price * trade.volume * self.commission_ratio
        if commission == self.min_commission:  # first trade
            if amt > commission:
                self._commission_map[order_id] = 0.0
                return amt
            else:
                self._commission_map[order_id] -= amt
                return self.min_commission
        else:
            if amt > commission:
                self._commission_map[order_id] = 0.0
                return amt - commission
            else:
                self._commission_map[order_id] -= amt
                return 0.0


class FuturePosition(Position):
    _INSTRUMENT_TYPES = [InstrumentType.Future]

    def __init__(self, ctx, book, **kwargs):
        super(FuturePosition, self).__init__(ctx, book, **kwargs)

        inst_info = self.ctx.get_inst_info(self.instrument_id)
        self.contract_multiplier = inst_info["contract_multiplier"]
        self.margin_ratio = inst_info["long_margin_ratio"] if self.direction == Direction.Long else inst_info["short_margin_ratio"]

        commission_info = self.ctx.get_commission_info(self.instrument_id)
        self.commission_mode = commission_info["mode"]
        self.open_commission_ratio = commission_info["open_ratio"]
        self.close_commission_ratio = commission_info["close_ratio"]
        self.close_today_commission_ratio = commission_info["close_today_ratio"]

        self.pre_settlement_price = kwargs.pop("pre_settlement_price", 0.0)
        self.settlement_price = kwargs.pop("settlement_price", 0.0)
        self.last_price = kwargs.pop("last_price", 0.0)
        self.avg_open_price = kwargs.pop("avg_open_price", 0.0)
        self.position_cost_price = kwargs.pop("position_cost_price", 0.0)

        self.volume = kwargs.pop("volume", 0)
        self.yesterday_volume = kwargs.pop("yesterday_volume", 0)

        self.frozen_total = kwargs.pop("frozen_total", 0)
        self.frozen_yesterday = kwargs.pop("frozen_yesterday", 0)

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
    def today_volume(self):
        return self.volume - self.yesterday_volume

    @property
    def frozen_today(self):
        return self.frozen_total - self.frozen_yesterday

    @property
    def volume_available(self):
        return self.volume - self.frozen_total

    @property
    def yesterday_volume_available(self):
        return self.yesterday_volume - self.frozen_yesterday

    @property
    def today_volume_available(self):
        return self.today_volume - self.today_frozen

    @property
    def event(self):
        data = pykungfu.longfist.types.Position()
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
        data.frozen_yesterday = self.frozen_yesterday
        data.frozen_total = self.frozen_total
        data.direction = self.direction
        data.margin = self.margin
        return self.book.make_event(wc_msg.Position, data)

    def apply_order_input(self, input):
        if input.offset == Offset.Open:
            frozen_margin = self.contract_multiplier * input.frozen_price * input.volume * self.margin_ratio
            if self.book.avail < frozen_margin:
                raise ValueError("{} to frozen for order {}, but {} avail for {}".
                                 format(frozen_margin, input.order_id, self.book.avail, self.book.location.uname))

            self.book.avail -= frozen_margin
            self.book.frozen_cash += frozen_margin
            self.book.frozen_margin += frozen_margin
        elif input.offset == Offset.Close or input.offset == Offset.CloseYesterday:
            if self.volume_available < input.volume:
                raise ValueError("{} to frozen for order {}, but volume avail is {} for {} {}".
                                 format(input.volume, input.order_id, self.volume_available, self.book.location.uname, self.uname))
            self.frozen_total += input.volume
            if self.yesterday_volume_available > input.volume:
                self.frozen_yesterday += input.volume
            else:
                self.frozen_yesterday = self.yesterday_volume
        elif input.offset == Offset.CloseToday:
            if self.today_volume_available < input.volume:
                raise ValueError("{} to frozen for order {}, but today volume avail is {} for {} {}".
                                 format(input.volume, input.order_id, self.today_volume_available, self.book.location.uname, self.uname))
            self.frozen_total += input.volume
        self.book.subject.on_next(self.event)

    def apply_order(self, order):
        if not order.status in [pykungfu.longfist.enum.OrderStatus.Submitted, pykungfu.longfist.enum.OrderStatus.Pending, pykungfu.longfist.enum.OrderStatus.PartialFilledActive] and order.volume_left > 0:
            if order.offset == Offset.Open:
                frozen_margin = self.contract_multiplier * order.frozen_price * order.volume * self.margin_ratio
                self.book.avail += frozen_margin
                self.book.frozen_cash -= frozen_margin
                self.book.frozen_margin -= frozen_margin
            elif order.offset == Offset.Close or order.offset == Offset.CloseYesterday:
                if self.frozen_total < order.volume_left:
                    raise ValueError("{} to release for order {}, but frozen is {} for {} {}".
                                     format(order.volume_left, order.order_id, self.volume_available, self.book.location.uname, self.uname))
                self.frozen_total -= order.volume_left
                self.frozen_yesterday = self.frozen_yesterday - order.volume_left if \
                    self.frozen_yesterday > order.volume_left else 0
            elif order.offset == Offset.CloseToday:
                if self.frozen_today < order.volume_left:
                    raise ValueError("{} to release for order {}, but frozen is {} for {} {}".
                                     format(order.volume_left, order.order_id, self.frozen_today, self.book.location.uname, self.uname))
                self.frozen_total -= order.volume_left
            self.book.subject.on_next(self.event)

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
        if self.frozen_total < trade.volume:
            raise ValueError("{} {} over close position, current frozen volume is {}, {} to close".
                             format(self.book.location.uname, self.uname, self.frozen_total, trade.volume))
        if trade.offset == Offset.CloseToday and self.frozen_today < trade.volume:
            raise ValueError("{} {} over close today position, current frozen volume is {}, {} to close".
                             format(self.book.location.uname, self.uname, self.volume - self.yesterday_volume, trade.volume))
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        delta_margin = margin if self.margin > margin else self.margin
        self.margin -= delta_margin
        self.book.avail += delta_margin
        self.pre_today_vol = self.today_volume
        self.volume -= trade.volume
        self.frozen_total -= trade.volume
        if trade.offset != Offset.CloseToday:
            self.yesterday_volume = 0 if self.yesterday_volume <= trade.volume else \
                self.yesterday_volume - trade.volume
            self.frozen_yesterday = 0 if self.frozen_yesterday <= trade.volume else \
                self.frozen_yesterday - trade.volume
        trade.close_today_volume = self.today_volume - self.pre_today_vol
        commission = self._calculate_commission(trade)
        realized_pnl = (trade.price - self.avg_open_price) * trade.volume * \
                       self.contract_multiplier * (1 if self.direction == Direction.Long else -1)
        trade.commission = commission
        self.realized_pnl += realized_pnl
        self.book.realized_pnl += realized_pnl
        self.book.avail -= commission
        self.book.accumulated_fee += commission
        self.book.intraday_fee += commission

    def _apply_open(self, trade):
        margin = self.contract_multiplier * trade.price * trade.volume * self.margin_ratio
        commission = self._calculate_commission(trade)
        trade.commission = commission
        self.margin += margin
        frozen_margin = self.contract_multiplier * self.book.get_frozen_price(trade.order_id) * trade.volume * self.margin_ratio
        self.book.frozen_cash -= frozen_margin
        self.book.frozen_margin -= frozen_margin
        self.book.avail -= commission
        self.book.accumulated_fee += commission
        self.book.intraday_fee += commission
        self.avg_open_price = (self.avg_open_price * self.volume + trade.volume * trade.price) / (self.volume + trade.volume)
        self.volume += trade.volume

    def _calculate_commission(self, trade):
        if self.commission_mode == CommissionRateMode.ByAmount:
            if trade.offset == Offset.Open:
                return trade.price * trade.volume * self.contract_multiplier * self.open_commission_ratio
            else:
                return trade.price * (trade.volume - trade.close_today_volume) * \
                       self.contract_multiplier * self.close_commission_ratio \
                       + trade.price * trade.close_today_volume * self.contract_multiplier * self.close_today_commission_ratio
        else:
            if trade.offset == Offset.Open:
                return trade.volume * self.contract_multiplier * self.open_commission_ratio
            else:
                return (trade.volume - trade.close_today_volume) * self.contract_multiplier * self.close_commission_ratio \
                       + trade.close_today_volume * self.contract_multiplier * self.close_today_commission_ratio
