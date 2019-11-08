
import pywingchun
from . import mdmaker
from collections import namedtuple
from kungfu.yijinjing.log import create_logger
import kungfu.yijinjing.time as kft
import pyyjj
import kungfu.wingchun.msg as wc_msg

MakerConfig = namedtuple("MakerConfig", ["base", "bound", "samples", "variation", "randseed"])

class MarketDataSim(pywingchun.MarketData):
    def __init__(self, low_latency, locator, config_json):
        pywingchun.MarketData.__init__(self, low_latency, locator, "sim")
        self.config = MakerConfig(base=200.0, bound=1000, samples=1000,variation=4, randseed=6)
        self.orderbooks = {}
        self.logger = create_logger("sim_md", "info", pyyjj.location( pyyjj.mode.LIVE, pyyjj.category.MD, "sim", "sim", locator))

    def on_start(self):
        self.add_time_interval(500 * 1000 * 1000, lambda e: self.update_orderbooks())
        pywingchun.MarketData.on_start(self)

    def quote_from_orderbook(self, ob):
        quote = pywingchun.Quote()
        instrument_id, exchange_id = ob.security.split(".")
        quote.data_time = self.now()
        quote.instrument_id = instrument_id
        quote.exchange_id = exchange_id
        quote.ask_price = [ob.offer_price(i) for i in range(0, min(10, ob.depth_offers()))]
        quote.ask_volume = [ob.offer_qty(i) for i in range(0, min(10, ob.depth_offers()))]
        quote.bid_price = [ob.bid_price(i) for i in range(0, min(10, ob.depth_bids()))]
        quote.bid_volume = [ob.bid_qty(i) for i in range(0, min(10, ob.depth_bids()))]
        quote.last_price = round((quote.ask_price[0] + quote.bid_price[0]) / 2.0, 2)
        return quote

    def init_order_book(self, instrument_id, exchange_id):
        security = instrument_id + "." + exchange_id
        symbol_id = pywingchun.utils.get_symbol_id(instrument_id, exchange_id)
        book = mdmaker.OrderBook(security=security)
        for i in range(mdmaker.MAX_DEPTH):
            delta = (i+1) * 1.0
            book.order(mdmaker.Order(secid=book.security,side=mdmaker.Side.BUY, price=(self.config.base - delta), qty=1))
            book.order(mdmaker.Order(secid=book.security,side=mdmaker.Side.SELL, price=(self.config.base + delta), qty=1))
        self.orderbooks[symbol_id] = book

    def update_orderbooks(self):
        for book in self.orderbooks.values():
            order_generator = book.gen_orders(self.config)
            for orders, mid in order_generator:
                for order in orders:
                    instrument_id, exchange_id = order.secid.split(".")
                    if not pywingchun.utils.get_instrument_type(instrument_id, exchange_id) == pywingchun.constants.InstrumentType.Future:
                        order.qty *= 100
                    trades = book.order(order)
            quote = self.quote_from_orderbook(book)
            self.get_writer(0).write_data(0, wc_msg.Quote, quote)

    def subscribe(self, instruments):
        for inst in instruments:
            symbol_id = pywingchun.utils.get_symbol_id(inst.instrument_id, inst.exchange_id)
            if symbol_id not in self.orderbooks:
                self.init_order_book(inst.instrument_id, inst.exchange_id)
        return True

    def unsubscribe(self, instruments):
        return False

