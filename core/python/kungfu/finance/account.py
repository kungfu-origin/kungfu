
from position import *
import pywingchun

class StockAccount:
    def __init__(self):
        self._initial_equity = 0.0
        self._tot_cash = 0.0
        self._realized_pnl = 0.0
        self._positions = {}

    @property
    def market_value(self):
        return sum([position.market_value for position in self._positions.values()])

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @property
    def dynamic_equity(self):
        return self._tot_cash + self.market_value

    @property
    def unrealized_pnl(self):
        return sum([position.unrealized_pnl for position in self._positions.values()])

    def update_last_price(self, instrument_id, exchange_id, last_price):
        self._get_position(instrument_id, exchange_id).update_last_price(last_price)

    def apply_trade(self, trade):
        position = self._get_position(trade.instrument_id, trade.exchange_id)
        position.apply_trade(trade)

    def _get_position(self, instrument_id, exchange_id):
        symbol = pywingchun.get_symbol_id(instrument_id, exchange_id)
        if not self.positions.has_key(symbol):
            self.positions[symbol] = StockPosition(instrument_id, exchange_id)
        return self.positions[symbol]

    def reset_positions(self):
        pass

class FutureAccount:
    def __init__(self):
        self._positions = {}

    def update_last_price(self, instrument_id, exchange_id, last_price):
        pass

    def apply_trade(self, trade):
        pass
