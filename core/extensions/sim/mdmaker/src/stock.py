#!/usr/bin/python3
"""
Market data generator script.
"""

import argparse
import csv
import itertools
import math
import random
import struct
import sys
import time
from enum import Enum
from functools import reduce

import numpy
from recordclass import recordclass
from sortedcontainers import SortedListWithKey

MIN_TICK = 0.01
MAX_TICK = 20 * MIN_TICK
DECIMALS = int(abs(math.log10(MIN_TICK)))
DECIMAL_CONVERT = math.pow(10, DECIMALS)
MAX_QUANTITY = 40
VAL_RATIO_MAX = 0.95
SPREAD_MAX = 20 * MIN_TICK
MAX_DEPTH = 20
MAX_QTY = 100

LEVEL_FIELDS = ['price', 'qty', 'order_count']
TRADE_FIELDS = ['price', 'qty', 'aggressor']
FIELDS = ['timestamp', 'secid', 'trade_valid', 'book_valid']

MIN_TIME_DELTA_NANOS = 200
MAX_TIME_DELTA_NANOS = 100000000

OrderBookLevel = recordclass('OrderBookLevel',
                             LEVEL_FIELDS)
Order = recordclass('Order', ['secid', 'side', 'price', 'qty'])
Trade = recordclass('Trade', TRADE_FIELDS)

LEVEL_FORMAT = "lvl{}_{}_{}"
TRADE_FORMAT = "trade_{}"

def now_nanos():
    """Returns the current simulation time in nanoseconds"""
    if now_nanos.sim_time == 0:
        now_nanos.sim_time += int(time.time() * 1000000000)
    else:
        now_nanos.sim_time += random.randint(MIN_TIME_DELTA_NANOS, MAX_TIME_DELTA_NANOS)

    return now_nanos.sim_time

class Side(Enum):
    """
        Trading Side
    """
    BUY = 1
    SELL = 2

class OrderBookUtils(object):
    """Utility class for OrderBook manipulation"""
    @staticmethod
    def cross(bid, offer):
        """
            Check if two levels can cross
        """
        if bid.price >= offer.price:
            remaining_qty = min(offer.qty - bid.qty, MAX_QTY)
            return (True, remaining_qty)

        return (False, 0)

    @staticmethod
    def compact(levels, start):
        """
            Compacts an order list, such that orders at the same price level are merged.
            This assumes the order list is sorted.
        """

        # print("Compacting book")
        # self.print()
        last_level = None
        for i in range(start, -1, -1):
            level = levels[i]
            if last_level:
                if level.price == last_level.price:
                    last_level.qty += level.qty
                    last_level.order_count += level.order_count
                    del levels[i]
                else:
                    break
            else:
                last_level = level

    @staticmethod
    def reduce_book_value(level1, level2):
        """
            Reduction function for price * qty
        """
        value1 = level1
        value2 = level2
        if isinstance(level1, OrderBookLevel):
            value1 = level1.price * level1.qty
        if isinstance(level2, OrderBookLevel):
            value2 = level2.price * level2.qty
        return value1 + value2

    @staticmethod
    def qty(levels, at_level):
        """Get qty at sepcific level"""
        if len(levels) > at_level:
            return levels[at_level].qty
        return 0

    @staticmethod
    def price(levels, at_level):
        """Get price at sepcific level"""
        if len(levels) > at_level:
            return levels[at_level].price
        return 0.0

    @staticmethod
    def book_value(levels):
        """
            Computes the value of open orders on a list of book levels
        """
        if len(levels) > 1:
            return reduce(OrderBookUtils.reduce_book_value, levels)
        elif len(levels) > 0:
            return levels[0].price * levels[0].qty
        else:
            raise Exception("Cannot reduce empty book")

    @staticmethod
    def pad_book(book, depth, price, side):
        """
            Add orders to the book such that we are at full depths.
        """
        orders = []
        sign = -1.0 if side == Side.BUY else 1.0
        if depth < MAX_DEPTH:
            num = MAX_DEPTH - depth
            best = price
            offset = 1
            for _ in itertools.repeat(None, num):
                orders += [Order(secid=book.security,
                                 side=side,
                                 price=round(best + sign * offset *
                                             MIN_TICK, DECIMALS),
                                 qty=random.randint(1, 10))]
                offset += random.randint(1, 3)
        return orders

    @staticmethod
    def csv_header(file):
        """
            Write the CSV header to the output file
        """
        fields = FIELDS + \
                 [LEVEL_FORMAT.format(1, 'bid', field) for field in reversed(LEVEL_FIELDS)] + \
                 [LEVEL_FORMAT.format(1, 'offer', field) for field in LEVEL_FIELDS] + \
                 [TRADE_FORMAT.format(field) for field in TRADE_FIELDS]
        csvfile = csv.DictWriter(f=file, fieldnames=fields)
        csvfile.writeheader()
        return csvfile

    @staticmethod
    def csv_book_update(csvfile, book):
        """
            Add row to CSV file with current book state.
        """
        row = {'timestamp' : now_nanos(),
               'secid' : book.security,
               'trade_valid' : False,
               'book_valid' : 'True'}
        bid = book.bid[-1]._asdict()
        offer = book.offer[0]._asdict()
        row.update({
            LEVEL_FORMAT.format(1, 'bid', f) : bid[f] for f in LEVEL_FIELDS
            })
        row.update({
            LEVEL_FORMAT.format(1, 'offer', f) : offer[f] for f in LEVEL_FIELDS
            })
        row.update({TRADE_FORMAT.format(f) : None for f in TRADE_FIELDS})
        csvfile.writerow(row)

    @staticmethod
    def csv_trade_update(csvfile, book, trades):
        """
            Add row to CSV file for the trades
        """
        for trade in trades:
            row = {'timestamp' : now_nanos(),
                   'secid' : book.security,
                   'trade_valid' : True,
                   'book_valid' : 'False'}
            trade_dict = trade._asdict()
            row.update({
                LEVEL_FORMAT.format(1, 'bid', f) : None for f in LEVEL_FIELDS
                })
            row.update({
                LEVEL_FORMAT.format(1, 'offer', f) : None for f in LEVEL_FIELDS
                })
            row.update({TRADE_FORMAT.format(f) : trade_dict[f] for f in TRADE_FIELDS})
            csvfile.writerow(row)

    @staticmethod
    def bin_book_update(binfile, book):
        """
            Adds a binary entry to the file
        """
        trade_update_fmt = "II"
        trade_update_data = [0, 0]
        order_book_level_fmt = "IIIIII"
        levels = [
            (book.bid[-(i+1)].price * DECIMAL_CONVERT,
             book.bid[-(i+1)].qty,
             book.bid[-(i+1)].order_count,
             book.offer[i].price * DECIMAL_CONVERT,
             book.offer[i].qty,
             book.offer[i].order_count) for i in range(5)]
        order_book_level_data = []
        for data in levels:
            order_book_level_data += list(data)
        order_book_level_data = [int(v) for v in order_book_level_data]
        valids_fmt = "I"
        valids_data = [2]
        the_data = [now_nanos(), book.security] + \
                   trade_update_data + order_book_level_data + valids_data
        data = struct.pack("<QI" + trade_update_fmt + order_book_level_fmt * 5 + valids_fmt,
                           *the_data)
        binfile.write(data)

    @staticmethod
    def bin_trade_update(binfile, book, trades):
        """
            Adds a binary entry to the file
        """
        for trade in trades:
            trade_update_fmt = "II"
            trade_update_data = [int(trade.price * DECIMAL_CONVERT), trade.qty]
            order_book_level_fmt = "IIIIII"
            levels = [(0, 0, 0, 0, 0, 0) for i in range(5)]
            order_book_level_data = []
            for data in levels:
                order_book_level_data += list(data)
            order_book_level_data = [int(v) for v in order_book_level_data]
            valids_fmt = "I"
            valids_data = [1]
            the_data = [now_nanos(), book.security] + \
                       trade_update_data + order_book_level_data + valids_data
            data = struct.pack("<QI" + trade_update_fmt + order_book_level_fmt * 5 + valids_fmt,
                               *the_data)
            binfile.write(data)


class OrderBook(object):
    """An Order Book data model"""
    def __init__(self, security):
        self.bid = SortedListWithKey(key=(lambda level: level.price))
        self.offer = SortedListWithKey(key=(lambda level: level.price))
        self.security = security
        self.mids = []
        self.last_price = 0.0

    def match(self, aggressor_side):
        """
            Match orders and return a list of trades
        """

        # print("Matching on the following book:")
        # self.print()
        trades = []
        for bid_i in range(len(self.bid) - 1, -1, -1):
            bid = self.bid[bid_i]
            size_offer = len(self.offer)
            offer_i = 0
            while offer_i < size_offer:
                offer = self.offer[offer_i]
                (crossed, remaining_qty) = OrderBookUtils.cross(bid, offer)
                if crossed:
                    trade = Trade(price=offer.price, qty=offer.qty, aggressor=aggressor_side)
                    self.last_price = offer.price
                    stop = False
                    if remaining_qty >= 0:
                        offer.qty = remaining_qty
                        trade.qty = bid.qty
                        del self.bid[bid_i]
                        stop = True
                    if remaining_qty <= 0:
                        bid.qty = abs(remaining_qty)
                        del self.offer[offer_i]
                        size_offer -= 1
                    else:
                        offer_i += 1
                    trades += [trade]
                    if stop:
                        break
                else:
                    return trades
        return trades

    def order(self, order):
        """
            Apply an order to the Order Book.
            Return a list of trades generated as a result.
        """

        #print("Evaluating order: ", order)
        if self.security != order.secid:
            raise ("Cannot place order for security "
                   "%s on book[%s]" % (order.security, self.security))

        levels = self.bid
        if order.side == Side.SELL:
            levels = self.offer

        new_level = OrderBookLevel(price=order.price, qty=order.qty, order_count=1)
        start_index = levels.bisect_right(new_level)
        # levels.insert(start_index, new_level)
        levels.add(new_level)
        OrderBookUtils.compact(levels, start_index)

        # Trim list
        if order.side == Side.SELL:
            # Delete from end of list - highest offers
            size = len(self.offer)
            if size > MAX_DEPTH:
                for _ in itertools.repeat(None, size - MAX_DEPTH):
                    del self.offer[-1]
        else:
            # Delete from start of list - lowest bids
            size = len(self.bid)
            if size > MAX_DEPTH:
                for _ in itertools.repeat(None, size - MAX_DEPTH):
                    del self.bid[0]

        return self.match(order.side)

    def mid(self):
        """
            Get mid price of top of Book
        """
        if self.bid and self.offer:
            return (self.bid[-1].price + self.offer[0].price) / 2.0

        raise Exception("No bids / offers!")

    def update_mid_series(self):
        """Adds current mid price to the mid series"""
        self.mids += [self.mid()]

    def spread(self):
        """
            Get the spread at the Top of Book
        """
        if self.bid and self.offer:
            return self.offer[0].price - self.bid[-1].price

        return 0

    def value_offers(self):
        """
            Returns the value of offers on the book
        """
        return OrderBookUtils.book_value(self.offer)

    def value_bids(self):
        """
            Returns the value of bids on the book
        """
        return OrderBookUtils.book_value(self.bid)

    def depth_offers(self):
        """
            Returns the number of offers on the book
        """
        return len(self.offer)

    def depth_bids(self):
        """
            Returns the number of bids on the book
        """
        return len(self.bid)

    def best_bid(self):
        """Get best bid"""
        if self.bid:
            return self.bid[-1].price
        return None

    def best_offer(self):
        """Get best offer"""
        if self.offer:
            return self.offer[0].price
        return None

    def bid_price(self, at_level):
        """Get price at sepcific level"""
        return OrderBookUtils.price(self.bid, -(at_level+1))

    def bid_qty(self, at_level):
        """Get qty at sepcific level"""
        return OrderBookUtils.qty(self.bid, -(at_level+1))

    def offer_price(self, at_level):
        """Get price at sepcific level"""
        return OrderBookUtils.price(self.offer, at_level)

    def offer_qty(self, at_level):
        """Get qty at sepcific level"""
        return OrderBookUtils.qty(self.offer, at_level)

    def aggregate_bid_qty(self, trade_price):
        """Sum of qty that would match a price"""
        qty = 0
        for i in range(len(self.bid)):
            if self.bid[-i].price >= trade_price:
                qty += self.bid[-i].qty
        return qty

    def aggregate_offer_qty(self, trade_price):
        """Sum of qty that would match a price"""
        qty = 0
        for i in range(len(self.offer)):
            # print("trade_price = {} offer[{}] = {}".format(trade_price, i, self.offer[i].price))
            if self.offer[i].price <= trade_price:
                qty += self.offer[i].qty
                # print("Running qty = {}".format(qty))
        return qty

    def display(self):
        """
            Prints the order book
        """
        size_bid = len(self.bid)
        size_offer = len(self.offer)
        print("Book[%s]: %d bids, %d offers --> mid @ %f"  % (self.security,
                                                              size_bid, size_offer, self.mid()))
        print("{0: ^32} | {1: ^32}".format("bid", "offer"))
        print("{0:^10},{1:^10},{2:^10} | {3:^10}, {4:^10}, {5:^10}".format(
            "count", "qty", "price", "price", "qty", "count"))

        empty_level = OrderBookLevel("-", "-", "-")
        for i in range(max(size_bid, size_offer)):
            bid = self.bid[-(i+1)] if i < size_bid else empty_level
            offer = self.offer[i] if i < size_offer else empty_level
            print("{0:^10},{1:^10},{2:^10} | {3:^10}, {4:^10}, {5:^10}".format(
                bid.order_count, bid.qty, bid.price, offer.price, offer.qty, offer.order_count))

    def gen_orders(self, config):
        """Generator function for the mid price"""
        upper_bound = config.base + config.bound * MIN_TICK
        lower_bound = config.base - config.bound * MIN_TICK
        mid = config.base + random.randint(-config.bound + 1, config.bound - 1) * MIN_TICK
        direction = 1.0

        for i in range(config.samples):
            if i % config.variation == 0:
                if mid >= upper_bound:
                    direction = -1.0
                elif mid <= lower_bound:
                    direction = 1.0
                elif random.randint(0, 1) == 0:
                    direction = -1.0
                else:
                    direction = 1.0

            mid += direction * random.randint(1, 10) * MIN_TICK

            # if mid <= lower_bound:
            #     mid = lower_bound

            # if mid >= upper_bound:
            #     mid = upper_bound

            orders = []
            sell_price = mid + MIN_TICK
            buy_price = mid - MIN_TICK
            if direction < 0:
                qty = self.aggregate_bid_qty(sell_price)
                orders += [Order(secid=self.security,
                                 side=Side.SELL,
                                 price=round(sell_price, DECIMALS),
                                 qty=qty + 1)]
                orders += [Order(secid=self.security,
                                 side=Side.BUY,
                                 price=round(buy_price, DECIMALS),
                                 qty=1)]
            else:
                qty = self.aggregate_offer_qty(buy_price)
                orders += [Order(secid=self.security,
                                 side=Side.BUY,
                                 price=round(buy_price, DECIMALS),
                                 qty=qty+1)]
                orders += [Order(secid=self.security,
                                 side=Side.SELL,
                                 price=round(sell_price, DECIMALS),
                                 qty=1)]

            orders += OrderBookUtils.pad_book(self, self.depth_bids(), buy_price, Side.BUY)
            orders += OrderBookUtils.pad_book(self, self.depth_offers(), sell_price, Side.SELL)

            yield (orders, mid)

def gen_book(config):
    """Generate order book"""
    print("Using random seed: ", config.randseed)
    random.seed(config.randseed)

    books = []
    for sec in config.instruments:
        books += [OrderBook(security=sec, file=config.outputfile)]

    if config.csv:
        OrderBookUtils.csv_header(config.outputfile)

    # Initialize books
    for book in books:
        for i in range(MAX_DEPTH):
            delta = (i+1) * 1.0
            book.order(Order(secid=book.security,
                             side=Side.BUY, price=(config.base - delta), qty=1))
            book.order(Order(secid=book.security,
                             side=Side.SELL, price=(config.base + delta), qty=1))

    rem_books = [book for book in books]
    generators = [book.gen_orders(config) for book in books]

    while generators:
        index = random.randint(0, len(generators)-1)
        book = rem_books[index]
        gen = generators[index]
        try:
            (orders, mid) = next(gen)
            # book.print()
            # print("Target mid --> {}".format(mid))
            for order in orders:
                # print("New order: ", order)
                trades = book.order(order)
                if config.csv:
                    OrderBookUtils.csv_trade_update(config.outputfile, book, trades)
                else:
                    OrderBookUtils.bin_trade_update(config.outputfile, book, trades)

            if config.csv:
                OrderBookUtils.csv_book_update(config.outputfile, book)
            else:
                OrderBookUtils.bin_book_update(config.outputfile, book)

            book.update_mid_series()
        except StopIteration:
            del generators[index]
            del rem_books[index]


    return [book.mids for book in books]

def parse_args(argv):
    """Parse command line arguments"""
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--samples", dest="samples",
                        help="Number of samples required",
                        action="store", type=int, default=1000)

    parser.add_argument("-b", "--base", dest="base",
                        help="Starting price",
                        action="store", type=float, default=200.0)

    parser.add_argument("-c", "--csv", dest="csv",
                        help="Use CSV format [default Binary]",
                        action="store_true", default=False)

    parser.add_argument("-i", "--instruments", dest="instruments",
                        help="Security IDs, space separated list", type=int,
                        nargs='+',
                        action="store", default=666666)

    parser.add_argument("-v", "--variation", dest="variation",
                        help="Change in direction - lower is more often",
                        action="store", type=int, default=4)

    parser.add_argument("-u", "--bound", dest="bound",
                        help="Price bound",
                        action="store", type=int, default=1000)

    parser.add_argument("-g", "--debug", dest="debug",
                        help="Enable debugging",
                        action="store_true", default=False)

    parser.add_argument("-r", "--randomseed", dest="randseed",
                        help="Random seed",
                        action="store", type=int, default=6)

    parser.add_argument("-o", "--output", dest="outputfile",
                        help="Output file name",
                        action="store", type=argparse.FileType(mode='wb'), default="md")

    return parser.parse_args(argv)

def main(argv):
    """ Market data generator entry point """
    print("Market Data Generator")

    now_nanos.sim_time = 0
    args = parse_args(argv)
    print("Output file '{}' in {} format".format(args.outputfile.name,
                                                 'CSV' if args.csv else 'binary'))

    if args.csv:
        name = args.outputfile.name
        args.outputfile.close()
        args.outputfile = open(name, "w")

    mids = gen_book(args)
    colors = ['blue', 'green', 'red', 'cyan', 'magenta', 'yellow', 'black']

    for (i, series) in enumerate(mids):
        pyplot.plot(range(args.samples), numpy.array(series), colors[i % len(colors)])
    pyplot.show()


if __name__ == "__main__":
    main(sys.argv[1:])
