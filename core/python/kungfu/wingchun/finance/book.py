
from kungfu.wingchun.finance.position import StockPosition, FuturePosition, Position
from kungfu.wingchun.finance.position import get_uid as get_position_uid
from kungfu.wingchun.utils import *
from kungfu.wingchun.constants import *
import datetime
from pyyjj import hash_str_32
import sys
import traceback
from collections import namedtuple

DATE_FORMAT = "%Y%m%d"
DEFAULT_CASH = 1e7

class AccountBookTags(namedtuple('AccountBookTags', 'ledger_category source_id account_id client_id uname uid')):
    def __new__(cls, ledger_category, source_id="", account_id="", client_id=""):
        uname = "{}|{}|{}|{}".format(int(ledger_category), source_id, account_id, client_id)
        uid = hash_str_32(uname)
        return super(AccountBookTags, cls).__new__(cls, int(ledger_category), source_id, account_id, client_id, uname, uid)

    def __hash__(self):
        return self.uid

    def __eq__(self, other):
        return self.uid == other.uid

    def __ne__(self, other):
        return not(self == other)

class AccountBook:
    def __init__(self, ctx, **kwargs):
        self._ctx = ctx        
        self._trading_day = kwargs.pop("trading_day", None)
        if self._trading_day is None:
            self._trading_day = self._ctx.trading_day
        elif isinstance(self._trading_day, str):
            self._trading_day = datetime.datetime.strptime(self._trading_day, DATE_FORMAT)
        self._initial_equity = kwargs.pop("initial_equity", 0.0)
        self._static_equity = kwargs.pop("static_equity", 0.0)
        self._avail = kwargs.pop("avail", 0.0)
        self._realized_pnl = kwargs.pop("realized_pnl", 0.0)
        self._positions = {}
        positions = kwargs.pop("positions", [])
        for pos in positions:
            if isinstance(pos, dict):
                pos = Position.factory(ctx=self._ctx, book=self, **pos)
            self._positions[pos.uid] = pos
        if self._initial_equity <= 0.0:
            self._initial_equity = self.dynamic_equity # fill initial equity
        if self._static_equity <= 0.0:
            self._static_equity = self.dynamic_equity
        self._tags = kwargs.pop("tags")
        self._callbacks = []

    @property
    def avail(self):
        return self._avail

    @avail.setter
    def avail(self, value):
        self._avail = value

    @property
    def trading_day(self):
        return self._trading_day

    @property
    def message(self):
        message = {
            "msg_type": int(MsgType.Asset),
            "data": {
                "trading_day": self.trading_day.strftime(DATE_FORMAT),
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
        self.fill_msg_tag(message)
        return message

    @property
    def detail_messages(self):
        messages = [self.message]
        for pos in self.positions:
            messages.append(pos.message)
            if isinstance(pos, FuturePosition):
                for detail in pos.details:
                    if detail.volume > 0: messages.append(detail.message)
        for message in messages:
            self.fill_msg_tag(message)
        return messages

    @property
    def positions(self):
        return list(self._positions.values())

    @property
    def margin(self):
        return sum([position.margin for position in self._positions.values()])

    @property
    def market_value(self):
        return sum([position.market_value for position in self._positions.values() if isinstance(position, StockPosition)])

    @property
    def initial_equity(self):
        return self._initial_equity

    @property
    def static_equity(self):
        return self._static_equity

    @property
    def dynamic_equity(self):
        total_value = self.avail
        for pos in self.positions:
            if isinstance(pos, FuturePosition):
                total_value += (pos.margin + pos.position_pnl)
            elif isinstance(pos, StockPosition):
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

    @property
    def uname(self):
        return self._tags.uname

    @property
    def uid(self):
        return self._tags.uid

    @property
    def tags(self):
        return self._tags

    def __repr__(self):
        return "%s(%r)" % (self.__class__, self.message["data"])

    def register_callback(self, callback):
        self._callbacks.append(callback)

    def dispatch(self, messages):
        for msg in messages:
            self.fill_msg_tag(msg)
        self._ctx.logger.debug("dispatch messages: %s for callbacks: %s", messages, self._callbacks)
        for cb in self._callbacks:
            cb(messages)

    def fill_msg_tag(self, message):
        message["data"].update(self._tags._asdict())

    def merge(self, book):
        self._ctx.logger.info("merge {} with {}".format(self, book))
        if self.realized_pnl == 0.0:
            self.realized_pnl = book.realized_pnl
        if self._static_equity == DEFAULT_CASH:
            self._static_equity = book.static_equity
        if self._initial_equity == DEFAULT_CASH:
            self._initial_equity = book.initial_equity
        self.avail = book.avail
        for symbol in set(self._positions.keys()).union(set(book._positions.keys())):
            l_pos = self._positions.pop(symbol, None)
            r_pos = book._positions.get(symbol, None)
            if l_pos and r_pos:
                self._ctx.logger.info("merge position {} with {}".format(l_pos, r_pos))
                l_pos.merge(r_pos)
                self._positions[symbol] = l_pos
            elif not l_pos and r_pos:
                r_pos.book = self
                self._positions[symbol] = r_pos
                self._ctx.logger.info("add position {}".format(r_pos))
            elif l_pos and not r_pos:
                self._ctx.logger.info("delete position {}".format(l_pos))
        self._ctx.logger.info("merged {}".format(self))
        return self

    def get_position(self, instrument_id, exchange_id, direction = Direction.Long):
        uid = get_position_uid(instrument_id, exchange_id, direction)
        return self._positions.get(uid, None)

    def apply_quote(self, quote):
        self._ctx.logger.debug("{} apply quote".format(self.uname))
        position = self.get_position(quote.instrument_id, quote.exchange_id, Direction.Long)
        if position is not None:
            position.apply_quote(quote)
        position = self.get_position(quote.instrument_id, quote.exchange_id, Direction.Short)
        if position is not None:
            position.apply_quote(quote)

    def apply_trade(self, trade):
        self._ctx.logger.info("{} apply trade[(trade_id){} (inst){} (price){} (volume){}]".format(self.uname, trade.trade_id, trade.instrument_id, trade.price, trade.volume))
        instrument_type = get_instrument_type(trade.instrument_id, trade.exchange_id)
        direction = get_position_effect(instrument_type, trade.side, trade.offset)
        try:
            self._get_position(trade.instrument_id, trade.exchange_id, direction).apply_trade(trade)
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self._ctx.logger.error('apply trade error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def apply_trading_day(self, trading_day):
        for pos in self._positions.values():
            pos.apply_trading_day(trading_day)
        if not self.trading_day == trading_day:
            self._ctx.logger.info("{} apply trading day, switch from {} to {}".format(self.uname, self.trading_day, trading_day))
            self._trading_day = trading_day
            self._static_equity = self.dynamic_equity
        else:
            self._ctx.logger.debug("{} receive duplicate trading_day message {}".format(self.uname, trading_day))

    def _get_position(self, instrument_id, exchange_id, direction = Direction.Long):
        uid = get_position_uid(instrument_id, exchange_id, direction)
        if uid not in self._positions:
            position = Position.factory(ctx = self._ctx, book = self, instrument_id = instrument_id, exchange_id = exchange_id, direction = direction)
            self._positions[uid] = position
        return self._positions[uid]

