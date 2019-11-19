import pywingchun
import pyyjj
import json
import http
import functools
import sys
import traceback
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
import kungfu.yijinjing.msg as yjj_msg
from kungfu.wingchun import msg
from kungfu.yijinjing.log import create_logger
from kungfu.data.sqlite.data_proxy import LedgerDB
import kungfu.wingchun.book as kwb
from kungfu.wingchun.calendar import Calendar
from kungfu.wingchun.constants import OrderStatus
import kungfu.wingchun.utils as wc_utils
import kungfu.msg.utils as msg_utils
import kungfu.wingchun.constants as wc_constants

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
        self.ctx.logger = create_logger("ledger", ctx.log_level, self.io_device.home)
        self.ctx.calendar = Calendar(ctx)
        self.ctx.db = LedgerDB(self.io_device.home, ctx.name)
        self.ctx.inst_infos = {inst["instrument_id"]: inst for inst in self.ctx.db.all_instrument_infos()}
        self.ctx.orders = {}
        self.ctx.trading_day = None
        self.ctx.books = {}
        self.ctx.now = self.now
        self.ctx.get_inst_info = self.get_inst_info

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
        for inst_dict in self.ctx.inst_infos.values():
            inst = pywingchun.Instrument()
            for attr, value in inst_dict.items():
                if hasattr(inst, attr) and attr != "raw_address":
                    setattr(inst,attr, value)
            writer.write_data(0, msg.Instrument, inst)
        writer.mark(0, msg.InstrumentEnd)

    def handle_asset_request(self, event, location):
        self.ctx.logger.info("handle asset request for {} [{:08x}] from [{:08x}] ".format(location.uname, location.uid, event.source))
        book = self._get_book(location)
        writer = self.get_writer(event.source)
        writer.write_data(0, book.event.msg_type, book.event.data)
        for position in book.positions:
            event = position.event
            writer.write_data(0, event.msg_type, event.data)
        position_end = pywingchun.PositionEnd(book.location.uid)
        writer.write_data(0, msg.PositionEnd, position_end)

    def on_app_location(self, trigger_time, location):
        self.ctx.logger.info("{} {} [{:08x}]".format(kft.strftime(trigger_time), location.uname, location.uid))
        if location.category == pyyjj.category.TD:
            tags = kwb.book.AccountBookTags.make_from_location(location)
            self.ctx.logger.info("mark orders status unknown for {}[{:08x}] with tags: {}".format(location.uname, location.uid, tags))
            orders = self.ctx.db.mark_orders_status_unknown(tags.source_id, tags.account_id)
            for order in orders:
                self.publish(json.dumps({"msg_type": msg.Order, "data": order}, cls = wc_utils.WCEncoder))
            book = self._get_book(location)
            book.subject.subscribe(self.on_book_event)
            self.book_context.add_book(location, book)
        elif location.category == pyyjj.category.STRATEGY:
            book = self._get_book(location)
            book.subject.subscribe(self.on_book_event)
            self.book_context.add_book(location, book)
        self.ctx.db.add_location(location)

    def on_trading_day(self, event, daytime):
        self.ctx.logger.info('on trading day %s', kft.to_datetime(daytime))
        trading_day = kft.to_datetime(daytime)
        if self.ctx.trading_day is not None and self.ctx.trading_day != trading_day:
            self.publish(json.dumps({'msg_type': msg.Calendar,'data': {'trading_day': '%s' % self.ctx.calendar.trading_day}}))
        self.ctx.trading_day = trading_day

    def on_quote(self, event, quote):
        pass

    def get_location(self, location_uid):
        if self.has_location(location_uid):
            return pywingchun.Ledger.get_location(self, location_uid)
        else:
            return self.ctx.db.get_location(self.ctx, location_uid)

    def on_book_event(self, event):
        self.ctx.logger.debug("book event received: {}".format(event))
        event = event.as_dict()
        self.ctx.db.on_book_event(event)
        self.publish(json.dumps(event, cls= wc_utils.WCEncoder))

    def on_order(self, event, order):
        self.ctx.logger.debug('on order %s', order)
        frame_as_dict = event.as_dict()
        source_location = self.get_location(event.source) # account location which send order report event
        dest_location = self.get_location(event.dest) # strategy location which receive order report event

        frame_as_dict["data"]["source_id"] = source_location.group
        frame_as_dict["data"]["account_id"] = source_location.name
        frame_as_dict["data"]["client_id"] = dest_location.name
        frame_as_dict["data"]["order_id"] = str(order.order_id)
        frame_as_dict["data"]["parent_id"] = str(order.parent_id)

        self.ctx.orders[order.order_id] = frame_as_dict

        self.ctx.db.add_order(**frame_as_dict["data"])
        self.publish(json.dumps(frame_as_dict, cls = wc_utils.WCEncoder))

    def on_trade(self, event, trade):
        self.ctx.logger.debug('on trade %s', trade)
        frame_as_dict = event.as_dict()
        source_location = self.get_location(event.source) # account location which send trade report event
        dest_location = self.get_location(event.dest) # strategy location which receive trade report event
        source_id = source_location.group
        account_id = source_location.name
        client_id = dest_location.name
        frame_as_dict["data"]["source_id"] = source_id
        frame_as_dict["data"]["account_id"] = account_id
        frame_as_dict["data"]["client_id"] = client_id
        frame_as_dict["data"]["order_id"] = str(trade.order_id)
        frame_as_dict["data"]["trade_id"] = str(trade.trade_id)
        if source_id == "xtp" and trade.order_id in self.ctx.orders:
            self.ctx.logger.debug("update order {} by trade".format(trade.order_id))
            order_record = self.ctx.orders[trade.order_id]
            order = order_record["data"]
            if not wc_utils.is_final_status(order["status"]):
                order["volume_left"] = order["volume_left"]- trade.volume
                order["volume_traded"] = order["volume_traded"]+ trade.volume
                order["status"] = OrderStatus.PartialFilledActive if order["volume_left"] > 0 else OrderStatus.PartialFilledNotActive
                self.ctx.db.add_order(**order)
                self.publish(json.dumps(order_record, cls = wc_utils.WCEncoder))
            else:
                self.ctx.logger.debug("order {} enter final status {}, failed to update".format(trade.order_id, order["status"]))
        self.ctx.db.add_trade(**frame_as_dict["data"])
        self.publish(json.dumps(frame_as_dict, cls = wc_utils.WCEncoder))

    def on_instruments(self, instruments):
        inst_list = list(set(instruments))
        if inst_list:
            dicts = [msg_utils.object_as_dict(inst) for inst in inst_list]
            self.ctx.db.set_instruments(dicts)
            self.ctx.inst_infos = {inst["instrument_id"]: inst for inst in dicts}

    def _dump_snapshot(self, data_frequency="minute"):
        for book in self.ctx.books.values():
            event = book.event.as_dict()
            event["msg_type"] = msg.AssetSnapshot
            tags = {"update_time": self.now(), "data_frequency": data_frequency}
            event["data"].update(tags)
            self.ctx.db.on_book_event(event)
            self.publish(json.dumps(event, cls = wc_utils.WCEncoder))

    def has_book(self, uid):
        return uid in self.ctx.books

    def pop_book(self, uid):
        #self.book_context.pop_book(uid)
        return self.ctx.books.pop(uid, None)

    def _get_book(self, location):
        if location.uid not in self.ctx.books:
            book_tags = kwb.book.AccountBookTags.make_from_location(location)
            book = self.ctx.db.load_book(ctx = self.ctx, location=location)
            if not book:
                self.ctx.logger.info("failed to load book from sqlite for {} [{:08x}]".format(location.uname, location.uid))
                book = kwb.book.AccountBook(self.ctx, location = location, avail=DEFAULT_INIT_CASH)
            self.ctx.books[location.uid] = book
            self.ctx.logger.info("success to init book for {} [{:08x}]".format(location.uname, location.uid))
        return self.ctx.books[location.uid]

    def get_inst_info(self, instrument_id):
        if not instrument_id in self.ctx.inst_infos:
            self.ctx.inst_infos[instrument_id] = self.ctx.db.get_instrument_info(instrument_id)
        return self.ctx.inst_infos[instrument_id]

@on(msg.Calendar)
def calendar_request(ctx, event, location, data):
    message = {
        'msg_type': msg.Calendar,
        'data': {
            'trading_day': ctx.calendar.trading_day.strftime("%Y%m%d"),
        }
    }
    ctx.ledger.publish(json.dumps(message))
    message.update({'status': http.HTTPStatus.OK})
    return message

@on(msg.BrokerStateRefresh)
def broker_state_refresh(ctx, event, location, data):
    ctx.ledger.publish_broker_states(event.gen_time)
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.BrokerStateRefresh
    }

@on(msg.NewOrderSingle)
def new_order_single(ctx, event, location, data):
    # ctx.ledger.new_order_single(event, location.uid)
    return {
        'status': http.HTTPStatus.OK,
        'msg_type': msg.NewOrderSingle
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
            return {'status': http.HTTPStatus.OK,'msg_type': msg.CancelOrder}
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            ctx.logger.error('failed to cancel order %s, error [%s] %s', order_id, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
            return {'status': http.HTTPStatus.NOT_FOUND,'msg_type': msg.CancelOrder}
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

@on(msg.QryAsset)
def qry_asset(ctx, event, location, data):
    ctx.logger.info("qry asset, input: {}".format(data))
    book_tags = kwb.book.AccountBookTags(ledger_category=data["ledger_category"], source_id=data["source_id"],account_id=data["account_id"],client_id=data["client_id"])
    if book_tags in ctx.books:
        message = ctx.books[book_tags].message
        message.update({'status': http.HTTPStatus.OK, 'msg_type': msg.QryAsset})
        return message
    else:
        return {
            'status': http.HTTPStatus.NOT_FOUND,
            'msg_type': msg.QryAsset
        }

@on(msg.PublishAllAssetInfo)
def publish_all_asset(ctx, event, location, data):
    ctx.logger.info("req publish all recorded asset info")
    for book in ctx.books.values():
        event = book.event.as_dict()
        ctx.ledger.publish(json.dumps(event, cls = wc_utils.WCEncoder))
    return {'status': http.HTTPStatus.OK, 'msg_type': msg.PublishAllAssetInfo}

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
        ctx.db.remove_book(uid=location.uid)
        return {
            'status': http.HTTPStatus.OK,
            'msg_type': msg.RemoveStrategy
        }

@on(yjj_msg.TradingDay)
def update_trading_day(ctx, event, location, data):
    ctx.logger.warn("switch trading day from UI for test")
    daytime = data["daytime"]
    ctx.ledger.on_trading_day(event, daytime)
    return {'status': http.HTTPStatus.OK, 'msg_type': yjj_msg.TradingDay}
