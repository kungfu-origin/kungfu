
import enum
from abc import ABCMeta, abstractmethod

class LedgerCategory(enum.Enum):
    StockAccount = 1
    FutureAccount = 2
    StockSubPortfolio = 3
    FutureSubPortfolio = 4
    Portfolio = 5

class Ledger(with_metaclass(ABCMeta)):
    def __init__(self):
        pass

    @property
    @abstractmethod
    def category(self):
        raise NotImplementedError

    @property
    @abstractmethod
    def uid(self):
        raise NotImplementedError

    @property
    @abstractmethod
    def positions(self):
        raise  NotImplementedError

    @property
    @abstractmethod
    def avail(self):
        raise NotImplementedError

    @property
    @abstractmethod
    def initial_equity(self):
        raise NotImplementedError

    @property
    @abstractmethod
    def static_equity(self):
        raise NotImplementedError

    @property
    @abstractmethod
    def dynamic_equity(self):
        raise NotImplementedError

    @abstractmethod
    def get_position(self, instrument_id, exchange_id, direction):
        raise NotImplementedError

    @abstractmethod
    def apply_trade(self, trade):
        raise NotImplementedError

    @abstractmethod
    def apply_quote(self, quote):
        raise NotImplementedError
