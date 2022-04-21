import kungfu
from kungfu.wingchun.constants import *
import math 
import time
from datetime import datetime

yjj = kungfu.__binding__.yijinjing
class orderTask:
    def __init__(self, vol, side, offset, price):
        self.price = price
        self.vol = vol
        self.side = side
        self.offset = offset

def pre_start(context):
    args = {}
    context.SOURCE = ""
    context.ACCOUNT =""
    context.EXCHANGE =""
    context.TICKER = "" 
    context.bid_price = 0.0
    context.ask_price = 0.0
    context.SIDE = Side.Drop
    context.OFFSET = Offset.Open
    context.volume_to_fill = 0
    context.steps_to_fill = 0
    context.VOLUME = 0
    context.orders = {} 
    context.has_quote = False
    context.MIN_VOL = int(0)
    context.log.info("arguments {}".format(context.arguments))
    argslist = context.arguments.split(';')
    for i, arg in enumerate(argslist):
        kvs = arg.split('=')
        if(len(kvs) == 2):
            args[kvs[0]]=kvs[1]
    context.log.info("args {}".format(args))
    sourceAccountList = args["accountId"].split('_')
    exchangeTicker= args["ticker"].split('_')
    context.STEPS = int(args["steps"])
    context.steps_to_fill = context.STEPS
    context.SIDE = Side(int(args["side"]))
    context.OFFSET = Offset(int(args["offset"]))
    context.VOLUME = int(args["volume"])
    context.volume_to_fill = context.VOLUME
    context.LASTSINGULARITY = args["lastSingularity"]
    context.LASTSINGULARITYNANOSECOND = int(args["lastSingularityMilliSecond"]) * 10**6
    context.MAX_LOT_BY_STEP = int(args["maxLotByStep"])
    trigger_time = args["triggerTime"]
    finish_time = args["finishTime"]
    context.START_TIME_IN_NANO = str_to_nanotime(trigger_time)
    context.FINISH_TIME_IN_NANO = str_to_nanotime(finish_time)
    if context.START_TIME_IN_NANO < context.now():
        raise ValueError("trigger time must greater than current time!")
    if context.START_TIME_IN_NANO >= context.FINISH_TIME_IN_NANO:
        raise ValueError("finish time must greater than trigger time!")
    if(len(sourceAccountList) == 2 and len(exchangeTicker) == 5):
        context.SOURCE = sourceAccountList[0]
        context.ACCOUNT = sourceAccountList[1]
        context.EXCHANGE = exchangeTicker[0]
        context.TICKER = exchangeTicker[1]
        context.add_account(context.SOURCE, context.ACCOUNT, 100000.0)
        context.subscribe(context.SOURCE, [context.TICKER], context.EXCHANGE)
        context.log.info(f"SOURCE {context.SOURCE} context.TICKER {context.TICKER} context.EXCHANGE {context.EXCHANGE}")
    context.log.info(f"STEPS {context.STEPS} context.SIDE {context.SIDE} context.OFFSET {context.OFFSET} context.VOLUME {context.VOLUME} context.SOURCE {context.SOURCE} context.ACCOUNT {context.ACCOUNT} context.EXCHANGE {context.EXCHANGE} context.TICKER {context.TICKER} context.LASTSINGULARITY {context.LASTSINGULARITY} context.START_TIME_IN_NANO {context.START_TIME_IN_NANO} context.FINISH_TIME_IN_NANO {context.FINISH_TIME_IN_NANO}")

def str_to_nanotime(tm) :
    if(tm.isdigit()) : #in milliseconds
        return int(tm) * 10**6
    else :
        year_month_day = time.strftime("%Y-%m-%d", time.localtime())
        ymdhms = year_month_day + ' ' + tm.split(' ')[1]
        timeArray = time.strptime(ymdhms, "%Y-%m-%d %H:%M:%S")
        nano = int(time.mktime(timeArray) * 10**9)
        return nano

def type_to_minvol(argument):
    switcher = {
        InstrumentType.Stock : int(100),
        InstrumentType.Future : int(1),
        InstrumentType.Bond : int(1),
        InstrumentType.StockOption : int(1),
        InstrumentType.Fund : int(1),
        InstrumentType.TechStock : int(100),
        InstrumentType.Index : int(1),
    }
    return switcher.get(argument, int(1))

def on_order(context, order):
    context.log.info("[on_order 0] {}".format(order))
    if order.status == OrderStatus.Filled :
        del context.orders[order.order_id]
    elif (order.status == OrderStatus.Cancelled) or (order.status == OrderStatus.PartialFilledNotActive):
        context.log.info("[on_order 1] {}".format(context.orders))
        del context.orders[order.order_id]
        context.log.info("[on_order 2] {}".format(context.orders))
        if (not context.orders) and context.steps_to_fill > 1 :
            orders_to_insert = make_plan(context, False)
            context.log.info("------------orders_to_insert---------------- total {} orders".format(len(orders_to_insert)))
            for item in orders_to_insert:
                context.log.info("now {} insert_order_task orders_to_insert {}".format(context.now(), item.__dict__))
            make_order(context, orders_to_insert)
    elif (order.status == OrderStatus.Error) :
        del context.orders[order.order_id] 

def on_trade(context, trade):
    context.log.info("[on_trade] {}".format(trade))
    if trade.order_id in context.orders:
        context.orders[trade.order_id] -= trade.volume
    context.volume_to_fill -= trade.volume

def print_datatime(context, info, nano) :
    date_time_for_nano = datetime.fromtimestamp(nano / (10**9))
    time_str = date_time_for_nano.strftime("%Y-%m-%d %H:%M:%S.%f")
    context.log.info("{} nano {} datetime {}".format(info, nano, time_str))

def on_quote(context, quote):
    context.bid_price = quote.bid_price[0]
    context.ask_price = quote.ask_price[0]
    context.realtime_quote = quote
    context.UPPER_LIMIT_PRICE = quote.upper_limit_price
    context.LOWER_LIMIT_PRICE = quote.lower_limit_price
    if context.MIN_VOL == 0 :
        context.log.info("instrument_id {} bid {} ask {} up {} down {}".format(quote.instrument_id, str(quote.bid_price)[1:-1], str(quote.ask_price)[1:-1], quote.upper_limit_price, quote.lower_limit_price))
        context.MIN_VOL = type_to_minvol(quote.instrument_type)
        context.MAX_LOT_BY_STEP = int(math.floor(context.MAX_LOT_BY_STEP / float(context.MIN_VOL)) * context.MIN_VOL)
        context.log.info("quote.instrument_type {} context.MIN_VOL {} context.MAX_LOT_BY_STEP {}".format(quote.instrument_type, context.MIN_VOL, context.MAX_LOT_BY_STEP))
        assert context.MAX_LOT_BY_STEP > 0

    if not context.has_quote :
        '''
        #for test
        context.START_TIME_IN_NANO = int(context.now()  + 2 * 1e9)
        context.FINISH_TIME_IN_NANO = int(context.START_TIME_IN_NANO  + 30 * 1e9)
        '''
        print_datatime(context, "on_quote context.START_TIME_IN_NANO", context.START_TIME_IN_NANO)
        print_datatime(context, "on_quote context.FINISH_TIME_IN_NANO", context.FINISH_TIME_IN_NANO)
        interval = int((context.FINISH_TIME_IN_NANO - context.START_TIME_IN_NANO) / (context.STEPS - 1))
        context.log.info(f"interval {interval}")
        for i in range(context.STEPS):
            if i == context.STEPS - 1:
                last_order_time = int(context.START_TIME_IN_NANO + interval * i - context.LASTSINGULARITYNANOSECOND) if context.LASTSINGULARITY == 'true' else int(context.START_TIME_IN_NANO + interval * i)
                last_order_cancel_time = int(context.START_TIME_IN_NANO + interval * i - (interval + context.LASTSINGULARITYNANOSECOND) / 2) if context.LASTSINGULARITY == 'true' else int(context.START_TIME_IN_NANO + interval * i - interval / 2)
                context.add_timer(last_order_cancel_time, lambda ctx, event: cancel_all_orders(ctx))
                context.add_timer(last_order_time, lambda ctx, event: insert_order_task(ctx, True))
                print_datatime(context, "add_timer for cancel_all_orders at last step", last_order_cancel_time) 
                print_datatime(context, "add_timer for insert_order_task at last step", last_order_time) 
            else:
                context.add_timer(int(context.START_TIME_IN_NANO + interval * i), lambda ctx, event: insert_order_task(ctx, False))
                print_datatime(context, "add_timer for insert_order_task", context.START_TIME_IN_NANO + interval * i) 
        context.has_quote = True

def insert_order_task(context, last_order):
    print_datatime(context, "execute insert_order_task", context.now())
    if (not last_order) and context.orders:
        cancel_all_orders(context)
    else :
        orders_to_insert = make_plan(context, last_order)
        context.log.info("------------orders_to_insert----------------")
        for item in orders_to_insert:
            context.log.info("now {} insert_order_task orders_to_insert {}".format(context.now(), item.__dict__))
        make_order(context, orders_to_insert)

def cancel_all_orders(context):
    context.log.info("[cancel_all_orders] {}".format(context.orders))
    for item in context.orders.keys():
        context.log.info(f"canecl order id {item}")
        context.cancel_order(item)
    
def make_order(context, orders):
    order_step = context.STEPS - context.steps_to_fill + 1
    for item in orders:
        order_id = context.insert_order(context.TICKER, context.EXCHANGE, context.ACCOUNT, item.price, item.vol, PriceType.Limit, item.side, item.offset)
        context.orders[order_id] = item.vol
        date_time_for_nano = datetime.fromtimestamp(context.now() / (10**9))
        time_str = date_time_for_nano.strftime("%Y-%m-%d %H:%M:%S.%f")
        pending_vol = int(0)
        for order_vol in context.orders.values():
            pending_vol += order_vol
        rest_vol = context.volume_to_fill - pending_vol
        context.log.info("[第{}步下单] 标的 {} 交易所 {} 账户 {} 价格 {} 数量 {} 方向 {} 开平 {} 行情 {} 时间 {} 剩余数量 {}".format(order_step, context.TICKER, context.EXCHANGE, context.ACCOUNT, item.price, item.vol, item.side, item.offset, context.realtime_quote, time_str, rest_vol))
    context.steps_to_fill -= 1
    context.log.info("[make_order] context.orders {}".format(context.orders))

def split_order(context, vol, side, offset, price, task_list):
    place_order_vol = int(0)
    while vol > context.MAX_LOT_BY_STEP :
        task_list.append(orderTask(context.MAX_LOT_BY_STEP, side, offset, price))
        vol -= context.MAX_LOT_BY_STEP
        place_order_vol += context.MAX_LOT_BY_STEP
    if vol > 0 :
        task_list.append(orderTask(vol, side, offset, price))
        place_order_vol += vol
    return place_order_vol

def calc_order_vol(context, pos, side, offset, price) :
    vol = int(0)
    task_list = []
    offset_vol = (pos.volume - pos.yesterday_volume) if offset == Offset.CloseToday else pos.yesterday_volume
    offset_tag = Offset.CloseToday if offset == Offset.CloseToday else  Offset.CloseYesterday
    if(offset_vol >= context.volume_to_fill / context.steps_to_fill):
        vol = int(math.floor(context.volume_to_fill / context.steps_to_fill / float(context.MIN_VOL)) * context.MIN_VOL)
        split_order(context, vol, side, offset_tag, price, task_list)
    elif(pos.volume >= context.volume_to_fill / context.steps_to_fill):
        if(offset_vol > 0):
            vol = int(math.floor(offset_vol / float(context.MIN_VOL)) * context.MIN_VOL)
            place_order_vol = split_order(context, vol, side, offset_tag, price, task_list)
        vol = int(math.floor((context.volume_to_fill / context.steps_to_fill - place_order_vol) / float(context.MIN_VOL)) * context.MIN_VOL)
        split_order(context, vol, side, Offset.Close, price, task_list)
    else:
        if(offset_vol > 0):
            vol = int(math.floor(offset_vol / float(context.MIN_VOL)) * context.MIN_VOL)
            place_order_vol = split_order(context, vol, side, offset_tag, price, task_list)
        vol = int(math.floor((pos.volume - place_order_vol) / float(context.MIN_VOL)) * context.MIN_VOL)
        place_order_vol += split_order(context, vol, side, Offset.Close, price, task_list)
        vol = int(math.floor((context.volume_to_fill / context.steps_to_fill - place_order_vol) / float(context.MIN_VOL)) * context.MIN_VOL)
        split_order(context, vol, side, Offset.Open, price, task_list)
    return task_list

def make_plan(context, last_order):
    book = context.get_account_book(context.SOURCE, context.ACCOUNT)
    vol = int(0)
    if last_order :
        pending_vol = int(0)
        for order_vol in context.orders.values():
            pending_vol += order_vol
        order_price = context.UPPER_LIMIT_PRICE if context.SIDE == Side.Buy else context.LOWER_LIMIT_PRICE
        context.volume_to_fill -=  pending_vol
    else :
        order_price = context.bid_price if context.SIDE == Side.Buy else context.ask_price
    if(context.OFFSET == Offset.Open):
        vol = int(math.floor(context.volume_to_fill / context.steps_to_fill / float(context.MIN_VOL)) * context.MIN_VOL)
        context.log.info(f"make_plan..................vol {vol}")
        task_list = []
        split_order(context, vol, context.SIDE, Offset.Open, order_price, task_list)
        return task_list
    elif(context.OFFSET == Offset.Close or context.OFFSET == Offset.CloseYesterday):
        if(context.SIDE == Side.Buy):#Close, CloseYesterday, Side.Buy
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseYesterday, order_price)
        else:#Close, CloseYesterday, Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseYesterday, order_price)
    else: # CloseToday, Side.Buy
        if(context.SIDE == Side.Buy):
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseToday, order_price)
        else:#CloseToday,Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseToday, order_price)
    return []