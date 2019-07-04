
from kungfu.finance.position import *
from kungfu.wingchun.utils import *
from kungfu.wingchun.constants import *
from kungfu.finance.ledger import *

class StockAccount(Ledger):
    def __init__(self, **kwargs):
        self._account_id = kwargs.pop("account_id")
        self._uid = str(self.category) + "." + self._account_id
        self._initial_equity = kwargs.pop("initial_equity", 0.0)
        self._avail = kwargs.pop("avail", 0.0)
        self._realized_pnl = kwargs.pop("realized_pnl", 0.0)
        self._positions = kwargs.pop("positions", {})
        for pos in self._positions.values():
            pos.ledger = self
        if self._initial_equity <= 0.0:
            self._initial_equity = self.dynamic_equity # fill initial equity

    @property
    def category(self):
        return LedgerCategory.StockAccount

    @property
    def account_id(self):
        return self._account_id

    @property
    def uid(self):
        return self._uid

    @property
    def avail(self):
        return self._avail

    @property
    def market_value(self):
        return sum([position.market_value for position in self._positions.values()])

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @realized_pnl.setter
    def realized_pnl(self, value):
        self._realized_pnl = value

    @property
    def dynamic_equity(self):
        return self._avail + self.market_value

    @property
    def unrealized_pnl(self):
        return sum([position.unrealized_pnl for position in self._positions.values()])
    
    def apply_quote(self, quote):
        if is_valid_price(quote.last_price):
            self._get_position(quote.instrument_id, quote.exchange_id).last_price = quote.last_price
                
    def apply_trade(self, trade):
        position = self._get_position(trade.instrument_id, trade.exchange_id)
        position.apply_trade(trade)

    def _get_position(self, instrument_id, exchange_id):
        symbol = get_symbol_id(instrument_id, exchange_id)
        if not self.positions.has_key(symbol):
            self.positions[symbol] = StockPosition(ledger = self, instrument_id = instrument_id, exchange_id = exchange_id)
        return self.positions[symbol]

