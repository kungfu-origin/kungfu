from pykungfu import longfist
from pykungfu import wingchun as pywingchun
from pykungfu import yijinjing as pyyjj
import json
import http
import functools
import sys
import traceback
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
import kungfu.wingchun.book as kwb
import kungfu.wingchun.utils as wc_utils
import kungfu.wingchun.constants as wc_constants
from kungfu.wingchun import msg
from kungfu.yijinjing.log import create_logger
from kungfu.data.sqlite.data_proxy import CommissionDB
from kungfu.wingchun.calendar import Calendar
from kungfu.wingchun.book import accounting

DEFAULT_INIT_CASH = 1e7
HANDLERS = dict()


def on(msg_type):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)

        HANDLERS[msg_type] = handler_wrapper
        return on

    return register_handler


def handle(msg_type, *args, **kwargs):
    if msg_type not in HANDLERS:
        args[0].logger.error("invalid msg_type %s", msg_type)
    return HANDLERS[msg_type](*args, **kwargs)


class Ledger(pywingchun.Ledger):
    def __init__(self, ctx):
        pywingchun.Ledger.__init__(self, ctx.locator, ctx.mode, ctx.low_latency)
        self.ctx = ctx
        self.ctx.ledger = self
        self.ctx.config_location = self.config_location
        self.ctx.logger = create_logger("ledger", ctx.log_level, self.io_device.home)
        self.ctx.calendar = Calendar(ctx)
        self.ctx.commission_infos = {commission["product_id"]: commission for commission in
                                     CommissionDB(self.ctx.config_location, "commission").all_commission_info()}
        self.ctx.orders = {}
        self.ctx.books = {}
        self.ctx.now = self.now
        self.ctx.get_inst_info = self.get_inst_info
        self.ctx.get_commission_info = self.get_commission_info

        accounting.setup_bookkeeper(self.ctx, self.bookkeeper)

    def pre_start(self):
        self.add_time_interval(1 * kft.NANO_PER_MINUTE, lambda e: self._dump_snapshot())

    def handle_request(self, event, msg):
        req = json.loads(msg)
        data = req['data']
        location = kfj.get_location_from_json(self.ctx, data)
        return json.dumps(handle(req['msg_type'], self.ctx, event, location, data))

    def handle_instrument_request(self, event):
        self.ctx.logger.info("handle instrument request from [{:08x}]".format(event.source))
        writer = self.get_writer(event.source)
        for inst in self.instruments.values():
            writer.write(event.gen_time, inst)
        writer.mark(event.gen_time, msg.InstrumentEnd)

    def handle_asset_request(self, event, location):
        self.ctx.logger.info("handle asset request for {} [{:08x}] from [{:08x}] ".format(location.uname, location.uid, event.source))
        book = self._get_book(location)
        writer = self.get_writer(event.source)
        writer.write(event.gen_time, book.event.data)
        for position in book.positions:
            event = position.event
            writer.write(event.gen_time, event.data)
        position_end = longfist.types.PositionEnd()
        position_end.holder_uid = book.location.uid
        writer.write(event.gen_time, position_end)

    def on_trading_day(self, event, daytime):
        self.ctx.trading_day = kft.to_datetime(daytime)

    def has_book(self, uid):
        return uid in self.ctx.books

    def pop_book(self, uid):
        self.book_manager.pop_book(uid)
        return self.ctx.books.pop(uid, None)

    def get_inst_info(self, instrument_id):
        return self.instruments[instrument_id]

    def get_commission_info(self, instrument_id):
        product_id = wc_utils.get_product_id(instrument_id).upper()
        return self.ctx.commission_infos[product_id]

    def _dump_snapshot(self):
        for book in self.ctx.books.values():
            self.dump_asset_snapshot(book.event.data)


@on(msg.NewOrderSingle)
def new_order_single(ctx, event, location, data):
    if ctx.ledger.has_writer(location.uid):
        try:
            input = pywingchun.OrderInput()
            input.instrument_id = data.get("instrument_id")
            input.exchange_id = data.get("exchange_id")
            input.limit_price = data.get("limit_price", 0.0)
            input.volume = data.get("volume")
            side = data.get("side", wc_constants.Side.Buy)
            input.side = side = wc_constants.Side(side) if isinstance(side, int) else side
            offset = data.get("offset", wc_constants.Offset.Open)
            input.offset = wc_constants.Offset(offset) if isinstance(offset, int) else offset
            hedge_flag = data.get("hedge_flag", wc_constants.HedgeFlag.Speculation)
            input.hedge_flag = wc_constants.HedgeFlag(hedge_flag) if isinstance(hedge_flag, int) else hedge_flag
            price_type = data.get("price_type", wc_constants.PriceType.Limit)
            input.price_type = wc_constants.PriceType(price_type) if isinstance(price_type, int) else price_type
            writer = ctx.ledger.get_writer(location.uid)
            input.order_id = writer.current_frame_uid()
            writer.write_data(0, msg.OrderInput, input)
            return {
                'status': http.HTTPStatus.OK,
                'msg_type': msg.NewOrderSingle,
                "order_id": str(input.order_id)
            }
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            ctx.logger.error('failed to insert order, error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
            return {'status': http.HTTPStatus.NOT_FOUND, 'msg_type': msg.NewOrderSingle}
    else:
        ctx.logger.error("can not insert order to {} [{:08x}]".format(location.uname, location.uid))
        return {
            "status": http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.NewOrderSingle,
        }


@on(msg.CancelOrder)
def cancel_order(ctx, event, location, data):
    ctx.logger.info('cancel account order request')
    order_id = int(data['order_id'])
    if order_id in ctx.orders:
        order_record = ctx.orders[order_id]
        dest = order_record["dest"]
        source = order_record["source"]
        ctx.logger.info('cancel account order dest: %s, source: %s, order: %s', order_record["dest"], order_record["source"], order_record['data'])
        try:
            ctx.ledger.cancel_order(event, source, order_id)
            return {'status': http.HTTPStatus.OK, 'msg_type': msg.CancelOrder}
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            ctx.logger.error('failed to cancel order %s, error [%s] %s', order_id, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
            return {'status': http.HTTPStatus.NOT_FOUND, 'msg_type': msg.CancelOrder}
    else:
        ctx.logger.error('can not cancel order %s from orders %s', order_id, list(ctx.orders.keys()))
        return {
            'status': http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.CancelOrder
        }


@on(msg.CancelAllOrder)
def cancel_all_order(ctx, event, location, data):
    ctx.logger.info('cancel all account order request')
    for order_id in ctx.orders:
        order_record = ctx.orders[order_id]
        if order_record['source'] == location.uid:
            ctx.logger.info('cancel account order %s', order_record['data'])
            ctx.ledger.cancel_order(event, location.uid, order_id)
        if order_record['dest'] == location.uid:
            source = order_record["source"]
            if ctx.ledger.has_location(source):
                ctx.logger.info('cancel strategy order %s', order_record['data'])
                ctx.ledger.cancel_order(event, source, order_id)
            else:
                ctx.logger.warn("failed to find location {}".format(source))
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.CancelAllOrder
    }


@on(msg.RemoveStrategy)
def remove_strategy(ctx, event, location, data):
    if location is None:
        ctx.logger.warn("location is None, data: {}".format(data))
        return {
            'status': http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.RemoveStrategy
        }
    elif ctx.ledger.has_location(location.uid):
        ctx.logger.warn("strategy is running, failed to delete")
        return {
            'status': http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.RemoveStrategy
        }
    else:
        ctx.ledger.pop_book(uid=location.uid)
        return {
            'status': http.HTTPStatus.OK,
            'msg_type': msg.RemoveStrategy
        }
