from . import marketdata
from . import trader


def td(vendor):
    return trader.TraderSim(vendor)


def md(vendor):
    return marketdata.MarketDataSim(vendor)
