from collections import namedtuple
from kungfu.yijinjing.log import create_logger
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj
from pykungfu import wingchun as wc
from . import mdmaker

MakerConfig = namedtuple(
    "MakerConfig", ["base", "bound", "samples", "variation", "randseed"]
)


class MarketDataSim(wc.MarketData):
    def __init__(self, low_latency, locator, config_json):
        wc.MarketData.__init__(self, low_latency, locator, "sim")
        self.config = MakerConfig(
            base=200.0, bound=1000, samples=1000, variation=4, randseed=6
        )
        self.orderbooks = {}
        self.logger = create_logger(
            "sim_md",
            "info",
            yjj.location(
                lf.enums.mode.LIVE, lf.enums.category.MD, "sim", "sim", locator
            ),
        )

    def on_start(self):
        self.add_time_interval(500 * 1000 * 1000, lambda e: self.update_orderbooks())
        self.update_broker_state(lf.enums.BrokerState.Ready)
        wc.MarketData.on_start(self)

    def quote_from_orderbook(self, ob):
        quote = lf.types.Quote()
        instrument_id, exchange_id = ob.security.split(".")
        quote.data_time = self.now()
        quote.instrument_id = instrument_id
        quote.exchange_id = exchange_id
        quote.ask_price = [
            ob.offer_price(i) for i in range(0, min(10, ob.depth_offers()))
        ]
        quote.ask_volume = [
            ob.offer_qty(i) for i in range(0, min(10, ob.depth_offers()))
        ]
        quote.bid_price = [ob.bid_price(i) for i in range(0, min(10, ob.depth_bids()))]
        quote.bid_volume = [ob.bid_qty(i) for i in range(0, min(10, ob.depth_bids()))]
        quote.last_price = round((quote.ask_price[0] + quote.bid_price[0]) / 2.0, 2)
        return quote

    def init_order_book(self, instrument_id, exchange_id):
        security = instrument_id + "." + exchange_id
        instrument_key = wc.utils.hash_instrument(instrument_id, exchange_id)
        book = mdmaker.OrderBook(security=security)
        for i in range(mdmaker.MAX_DEPTH):
            delta = (i + 1) * 1.0
            book.order(
                mdmaker.Order(
                    secid=book.security,
                    side=mdmaker.Side.BUY,
                    price=(self.config.base - delta),
                    qty=1,
                )
            )
            book.order(
                mdmaker.Order(
                    secid=book.security,
                    side=mdmaker.Side.SELL,
                    price=(self.config.base + delta),
                    qty=1,
                )
            )
        self.orderbooks[instrument_key] = book
        self.logger.info(f"init order book for {instrument_id}@{exchange_id}")

    def update_orderbooks(self):
        for book in self.orderbooks.values():
            order_generator = book.gen_orders(self.config)
            for orders, mid in order_generator:
                for order in orders:
                    instrument_id, exchange_id = order.secid.split(".")
                    if (
                        not wc.utils.get_instrument_type(exchange_id, instrument_id)
                        == lf.enums.InstrumentType.Future
                    ):
                        order.qty *= 100
                    trades = book.order(order)
            quote = self.quote_from_orderbook(book)
            self.get_writer(0).write(0, quote)

    def subscribe(self, instruments):
        self.logger.info(f"subscribe {instruments}")
        for inst in instruments:
            instrument_key = wc.utils.hash_instrument(
                inst.instrument_id, inst.exchange_id
            )
            if instrument_key not in self.orderbooks:
                self.init_order_book(inst.instrument_id, inst.exchange_id)
        return True

    def unsubscribe(self, instruments):
        return False
