
from kungfu.finance.position import *
from kungfu.wingchun.utils import *
from kungfu.wingchun.constants import *

class Ledger:
    def __init__(self, **kwargs):
        self._trading_day = kwargs.pop("trading_day")
        self._initial_equity = kwargs.pop("initial_equity", 0.0)
        self._static_equity = kwargs.pop("static_equity", 0.0)
        self._avail = kwargs.pop("avail", 0.0)

        self._realized_pnl = kwargs.pop("realized_pnl", 0.0)
        self._positions = kwargs.pop("positions", {})
        for pos in self._positions.values():
            pos.ledger = self
        if self._initial_equity <= 0.0:
            self._initial_equity = self.dynamic_equity # fill initial equity
        if self._static_equity <= 0.0:
            self._static_equity = self.dynamic_equity

        self._category = kwargs.pop("ledger_category", LedgerCategory.Unknown)
        self._account_id = kwargs.pop("account_id", None)
        self._client_id = kwargs.pop("client_id", None)
        self._source_id = kwargs.pop("source_id", None)

        self._callbacks = []

    def register_callback(self, callback):
        self._callbacks.append(callback)

    def dispatch(self, messages):
        for cb in self._callbacks:
            cb(messages)

    @property
    def category(self):
        return self._category

    @property
    def account_id(self):
        return self._account_id

    @property
    def client_id(self):
        return self._client_id

    @property
    def source_id(self):
        return self._source_id

    @property
    def avail(self):
        return self._avail

    @avail.setter
    def avail(self, value):
        self._avail = value

    @property
    def msg_type(self):
       return int(MsgType.AssetInfo)

    @property
    def trading_day(self):
        return self._trading_day

    @property
    def message(self):
        return {
            "msg_type": self.msg_type,
            "data": {
                "ledger_category": int(self.category),
                "trading_day": self.trading_day.strftime("%Y%m%d"),
                "account_id": self.account_id,
                "client_id": self.client_id,
                "source_id": self.source_id,
                "avail": self.avail,
                "margin": self.margin,
                "market_value": self.market_value,
                "initial_equity": self.initial_equity,
                "dynamic_equity": self.dynamic_equity,
                "static_equity": self.static_equity,
                "realized_pnl": self.realized_pnl,
                "unrealized_pnl": self.unrealized_pnl
            }
        }

    @property
    def positions(self):
        return self._positions.values()

    @property
    def margin(self):
        return sum([position.margin for position in self._positions.values()])

    @property
    def market_value(self):
        return sum([position.market_value for position in self._positions.values()])

    @property
    def initial_equity(self):
        return self._initial_equity

    @property
    def static_equity(self):
        return self._static_equity

    @property
    def dynamic_equity(self):
        total_value = self.avail
        for pos in self._positions.values():
            if pos.instrument_type == InstrumentType.Future:
                total_value += (pos.margin + pos.position_pnl)
            else:
                total_value += pos.market_value
        return total_value

    @property
    def realized_pnl(self):
        return self._realized_pnl

    @realized_pnl.setter
    def realized_pnl(self, value):
        self._realized_pnl = value

    @property
    def unrealized_pnl(self):
        return sum([position.unrealized_pnl for position in self._positions.values()])
    
    def apply_quote(self, quote):
        self._get_position(quote.instrument_id, quote.exchange_id).apply_quote(quote)
                
    def apply_trade(self, trade):
        self._get_position(trade.instrument_id, trade.exchange_id).apply_trade(trade)

    def apply_trading_day(self, trading_day):
        if not self.trading_day == trading_day:
            self._trading_day = trading_day
            for pos in self._positions.values():
                pos.switch_day(trading_day)
            self._static_equity = self.dynamic_equity
            self.dispatch([self.message])

    def _get_position(self, instrument_id, exchange_id):
        symbol_id = get_symbol_id(instrument_id, exchange_id)
        if symbol_id not in self._positions:
            instrument_type = get_instrument_type(instrument_id, exchange_id)
            cls = StockPosition if instrument_type == InstrumentType.Stock else FuturePosition
            self._positions[symbol_id] = cls(ledger = self, instrument_id = instrument_id, exchange_id = exchange_id, instrument_type = instrument_type)
        return self._positions[symbol_id]

