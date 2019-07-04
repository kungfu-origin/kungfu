
from kungfu.wingchun.constants import *
from pyyjj import hash_str_32

def get_symbol_id(instrument_id, exchange_id, direction = DirectionLong):
    if isinstance(direction, int):
        direction = Direction(direction)
    return hash_str_32("{}.{}.{}".format(instrument_id, exchange_id, direction))

class StockPosition:
    def __init__(self, **kwargs):
        self._ledger = kwargs.pop("ledger", None)
        self._instrument_id = kwargs.pop("instrument_id")
        self._exchange_id = kwargs.pop("exchange_id")
        self._symbol_id = get_symbol_id(self._instrument_id, self._exchange_id)
        self._volume = kwargs.pop("volume", 0)
        self._yesterday_volume = kwargs.pop("yesterday_volume", 0)

        self._avg_open_price = kwargs.pop("open_price", 0.0)
        if self._avg_open_price <= 0.0:
            self._avg_open_price = kwargs.pop("cost_price", 0.0) # fill with position cost price
        self._last_price = kwargs.pop("last_price", 0.0)
        self._close_price = kwargs.pop("close_price", 0.0)
        self._pre_close_price = kwargs.pop("pre_close_price", 0.0)

        self._realized_pnl = kwargs.pop("realized_pnl", 0.0)

    @property
    def ledger(self):
        return self._ledger

    @ledger.setter
    def ledger(self, value):
        self._ledger = value

    @property
    def symbol_id(self):
        return self._symbol_id

    @property
    def instrument_id(self):
        return self._instrument_id

    @property
    def exchange_id(self):
        return self._exchange_id

    @property
    def last_price(self):
        return self._last_price

    @property
    def close_price(self):
        return self._close_price

    @property
    def pre_close_price(self):
        return self._pre_close_price

    @property
    def volume(self):
        return self._volume

    @property
    def market_value(self):
        return self.volume * (self.last_price if self.last_price > 0.0 else self.avg_open_price)

    @property
    def avg_open_price(self):
        return self._avg_open_price

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @property
    def unrealized_pnl(self):
        return (self.last_price - self.avg_open_price) * self.volume if self.last_price > 0.0 else 0.0

    def apply_trade(self, trade):
        if trade.side == Side.Buy:
            return self._apply_buy(trade.price, trade.volume)
        else:
            return self._apply_sell(trade.price, trade.volume)

    def apply_settlement(self, close_price):
        self.close_price = close_price

    def switch_day(self, trading_day):
        self.pre_close_price = self.close_price
        self.close_price = 0.0

    def _apply_sell(self, price, volume):
        realized_pnl = self._calculate_realized_pnl(price, volume)
        self.realized_pnl += realized_pnl
        self.ledger.realized_pnl += realized_pnl
        self.ledger.avail += (realized_pnl + price * volume)

    def _apply_buy(self, price, volume):
        self.avg_open_price = (self.avg_open_price * self.volume + price * volume) / (self.volume + volume)
        self.volume += volume
        self.ledger.avail -= price * volume

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self.avg_open_price) * trade_volume

class FuturePositionDetail:
    def __init__(self, **kwargs):
        self._open_price = kwargs.pop("open_price", 0.0)
        self._open_date = kwargs.pop("open_date", None)
        self._direction = kwargs.pop("direction", None)
        self._volume = kwargs.pop("volume", None)
        self._trading_day = kwargs.pop("trading_day", None)
        self._contract_multiplier = kwargs.pop("contract_multiplier", None)
        self._margin_ratio = kwargs.pop("margin_ratio", None)
        self._margin = 0.0
        self._settlement_price = None
        self._pre_settlement_price = None

    @property
    def open_date(self):
        return self._open_date

    @property
    def trading_day(self):
        return self._trading_day

    @property
    def open_price(self):
        return self._open_price

    @property
    def margin(self):
        if self.trading_day == self.open_date:
            if self.settlement_price is not None:
                return self._calculate_margin(self.settlement_price, self.volume)
            else:
                return self._calculate_margin(self.open_price, self.volume)
        else:
            return self._calculate_margin(self.pre_settlement_price, self.volume)

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
    def open_price(self):
        return self._open_price

    def apply_close(self, price, volume):
        volume_closed = min(self.volume, volume)
        realized_pnl = self._calculate_realized_pnl(price, volume_closed)
        margin_price = self.open_price if self.open_price == self.trading_day else self.pre_settlement_price
        margin_release = self._calculate_margin(margin_price, volume_closed)
        return (realized_pnl, margin_release, volume_closed)

    def apply_settlement(self, settlement_price):
        pre_margin = self.margin
        self.settlement_price = settlement_price
        return self.pre_margin - self.margin

    def _calculate_realized_pnl(self, trade_price, trade_volume):
        return (trade_price - self._open_price) * trade_volume * self._contract_multiplier * self._direction

    def _calculate_margin(self, price, volume):
        return self._contract_multiplier * price * volume * self._margin_ratio

class FuturePosition:
    def __init__(self, ctx, instrument_id, exchange_id, direction):
        self._instrument_id = instrument_id
        self._exchange_id = exchange_id
        self._direction = direction
        self._volume = 0
        self._yesterday_volume = 0
        self._pre_settlement_price = 0.0
        self._last_price = 0.0
        self._realized_pnl = 0.0
        self._trading_day = ctx.trading_day #TODO
        self._contract_multiplier = ctx.data_proxy.get_contract_multiplier(instrument_id, exchange_id)
        self._margin_ratio = ctx.data_proxy.get_margin_ratio(instrument_id, exchange_id, direction)
        self._details = []

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @property
    def last_price(self):
        return self._last_price

    @property
    def volume(self):
        return self._volume

    @property
    def direction(self):
        return self._direction

    def apply_close(self, trade):
        cash_delta = 0.0
        if trade.exchange_id == Exchange.SHFE and trade.offset == Offset.CloseToday:
            #TODO close today
            pass
        else:
            volume_left = trade.volume
            while volume_left > 0 and not self._details.empty():
                if self._details.empty():
                    break
                else:
                    detail = self._details[0]
                    realized_pnl, margin_release, volume_closed = detail.apply_close(trade.price, volume_left)
                    self.realized_pnl += self.realized_pnl
                    self.volume -= volume_closed
                    cash_delta += (realized_pnl + margin_release)
                    volume_left -= volume_closed
                    if detail.volume <= 0:
                        self._details.pop()
        return cash_delta

    def apply_open(self, trade):
        self.volume += trade.volume
        pass

    def apply_settlement(self, settlement_price):
        cash_delta = sum([ detail.apply_settlement(settlement_price) for detail in self._details])
        return cash_delta
