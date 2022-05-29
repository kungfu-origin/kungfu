import kungfu
from kungfu.wingchun.constants import *
import math 
import threading
import time
import json
from datetime import datetime

yjj = kungfu.__binding__.yijinjing
class orderTask:
    def __init__(self, vol, side, offset, price):
        self.price = price
        self.vol = vol
        self.side = side
        self.offset = offset

def pre_start(context):
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
    context.log.info("参数 {}".format(context.arguments))
    args = json.loads(context.arguments)
    sourceAccountList = args["accountId"].split('_')
    md = args["marketSource"]
    exchangeTicker= args["ticker"].split('_')
    context.STEPS = int(args["steps"])
    context.steps_to_fill = context.STEPS
    context.SIDE = Side(int(args["side"]))
    context.OFFSET = Offset(int(args["offset"]))
    context.VOLUME = int(args["volume"])
    context.volume_to_fill = context.VOLUME
    context.LASTSINGULARITY = args.get("lastSingularity", 'false')
    context.LASTSINGULARITYNANOSECOND = int(args.get("lastSingularityMilliSecond", '200')) * 10**6
    context.MAX_LOT_BY_STEP = int(args["maxLotByStep"])
    trigger_time = args["triggerTime"]
    finish_time = args["finishTime"]
    context.START_TIME_IN_NANO = str_to_nanotime(trigger_time)
    context.FINISH_TIME_IN_NANO = str_to_nanotime(finish_time)
    context.cancel_and_place_new_order = False
   # if context.START_TIME_IN_NANO < context.now():
   #     raise ValueError("trigger time must greater than current time!")
    if context.START_TIME_IN_NANO >= context.FINISH_TIME_IN_NANO:
        context.log.error("finish time must greater than trigger time!")
        context.req_deregister()
        return

    if(len(sourceAccountList) == 2 and len(exchangeTicker) == 5):
        context.SOURCE = sourceAccountList[0]
        context.ACCOUNT = sourceAccountList[1]
        context.EXCHANGE = exchangeTicker[0]
        context.TICKER = exchangeTicker[1]
        context.add_account(context.SOURCE, context.ACCOUNT, 100000.0)
        context.subscribe(md, [context.TICKER], context.EXCHANGE)
        context.log.info(f"MARKET SOURCE {md} context.TICKER {context.TICKER} context.EXCHANGE {context.EXCHANGE}")
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
        InstrumentType.TechStock : int(200),
        InstrumentType.Index : int(1),
    }
    return switcher.get(argument, int(1))

def on_order(context, order):
    context.log.info("[订单回调] {}".format(order))
    if order.status == OrderStatus.Filled :
        del context.orders[order.order_id]
    elif (order.status == OrderStatus.Cancelled) or (order.status == OrderStatus.PartialFilledNotActive):
        context.log.info("[on_order 1] {}".format(context.orders))
        del context.orders[order.order_id]
        context.log.info("[on_order 2] {}".format(context.orders))
        if (not context.orders) and context.steps_to_fill > 1 and context.cancel_and_place_new_order:
            context.cancel_and_place_new_order = False
            orders_to_insert = make_plan(context, False)
            context.log.info("------------准备下单---------------- 共计 {} 订单".format(len(orders_to_insert)))
            for item in orders_to_insert:
                context.log.info("时间戳 {} 订单 {}".format(context.now(), item.__dict__))
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
    context.log.info("{} 时间戳 {} 时间 {}".format(info, nano, time_str))

def add_job(context, nano_ts, func, args):
    now = context.now()
    if nano_ts > now:
        interval = (nano_ts - now) / (10**9)
        job = threading.Timer(interval, func, args)
        job.start()
        return job

def on_quote(context, quote):
    context.bid_price = quote.bid_price[0]
    context.ask_price = quote.ask_price[0]
    context.realtime_quote = quote
    context.UPPER_LIMIT_PRICE = quote.upper_limit_price
    context.LOWER_LIMIT_PRICE = quote.lower_limit_price
    if context.MIN_VOL == 0 :
        context.log.info("标的 {} 买 {} 卖 {} 涨停 {} 跌停 {}".format(quote.instrument_id, str(quote.bid_price)[1:-1], str(quote.ask_price)[1:-1], quote.upper_limit_price, quote.lower_limit_price))
        context.MIN_VOL = type_to_minvol(quote.instrument_type)
        assert context.MAX_LOT_BY_STEP >= context.MIN_VOL
        context.MAX_LOT_BY_STEP = int(math.floor(context.MAX_LOT_BY_STEP / float(context.MIN_VOL)) * context.MIN_VOL)
        context.log.info("标的类型 {} 最小交易数量 {} 单次最大数 {}".format(quote.instrument_type, context.MIN_VOL, context.MAX_LOT_BY_STEP))
        assert context.MAX_LOT_BY_STEP > 0

    if not context.has_quote :
        '''
        #for test
        context.START_TIME_IN_NANO = int(context.now()  + 2 * 1e9)
        context.FINISH_TIME_IN_NANO = int(context.START_TIME_IN_NANO  + 30 * 1e9)
        '''
        print_datatime(context, "起始时间戳", context.START_TIME_IN_NANO)
        print_datatime(context, "结束时间戳", context.FINISH_TIME_IN_NANO)
        if context.STEPS == 1:
            interval = int(context.FINISH_TIME_IN_NANO - context.START_TIME_IN_NANO)
        else:
            interval = int((context.FINISH_TIME_IN_NANO - context.START_TIME_IN_NANO) / (context.STEPS - 1))
        context.log.info(f"时间间隔(纳秒) {interval}")
        for i in range(context.STEPS):
            if i == context.STEPS - 1:
                ctx_now = context.now()
                last_order_time = int(context.START_TIME_IN_NANO + interval * i - context.LASTSINGULARITYNANOSECOND) if context.LASTSINGULARITY == 'true' else int(context.START_TIME_IN_NANO + interval * i)
                last_order_cancel_time = int(context.START_TIME_IN_NANO + interval * i - (interval + context.LASTSINGULARITYNANOSECOND) / 2) if context.LASTSINGULARITY == 'true' else int(context.START_TIME_IN_NANO + interval * i - interval / 2)
                if ctx_now < last_order_cancel_time :
                    add_job(context, last_order_cancel_time, cancel_all_orders, [context])
                    add_job(context, last_order_time, insert_order_task, [context, True])
                    print_datatime(context, "最后一步取消所有订单时间", last_order_cancel_time) 
                    print_datatime(context, "最后一步下单时间", last_order_time)
                elif ctx_now >= last_order_cancel_time and ctx_now < last_order_time:
                    last_order_cancel_time = int(ctx_now + (last_order_cancel_time - ctx_now) / 10)
                    add_job(context, last_order_cancel_time, cancel_all_orders, [context])
                    add_job(context, last_order_time, insert_order_task, [context, True])
                    print_datatime(context, "最后一步取消所有订单时间", last_order_cancel_time) 
                    print_datatime(context, "最后一步下单时间", last_order_time)
                else:
                    context.steps_to_fill -= 1
                    context.log.info("最后一步时间超时,退出任务...")
                    context.req_deregister()
            else:
                step_time = int(context.START_TIME_IN_NANO + interval * i)
                if context.now() < step_time :
                    add_job(context, step_time, insert_order_task, [context, False])
                    print_datatime(context, "添加定时任务", context.START_TIME_IN_NANO + interval * i) 
                else:
                    context.steps_to_fill -= 1
        context.has_quote = True

def insert_order_task(context, last_order):
    print_datatime(context, "执行定时任务", context.now())
    if (not last_order) and context.orders:
        context.cancel_and_place_new_order = True
        cancel_all_orders(context)
    else :
        orders_to_insert = make_plan(context, last_order)
        context.log.info("------------即将下订单----------------")
        for item in orders_to_insert:
            context.log.info("时间戳 {} 订单信息 {}".format(context.now(), item.__dict__))
        make_order(context, orders_to_insert)

def cancel_all_orders(context):
    context.log.info("[取消所有订单] {}".format(context.orders))
    for item in context.orders.keys():
        context.log.info(f"取消订单 订单id: {item}")
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
    context.log.info("[下单] {}".format(context.orders))
    if context.steps_to_fill == 0:
        context.log.info("任务完成")
        context.req_deregister()

def split_order(context, vol, side, offset, price, task_list, last_order):
    place_order_vol = int(0)
    while vol > context.MAX_LOT_BY_STEP :
        task_list.append(orderTask(context.MAX_LOT_BY_STEP, side, offset, price))
        vol -= context.MAX_LOT_BY_STEP
        place_order_vol += context.MAX_LOT_BY_STEP
    if vol > 0 :
        if last_order:
            vol_last_split = int(math.ceil(vol / float(context.MIN_VOL)) * context.MIN_VOL)
        else:
            vol_last_split = int(math.floor(vol / float(context.MIN_VOL)) * context.MIN_VOL)
        if vol_last_split > 0:
            task_list.append(orderTask(vol_last_split, side, offset, price))
            place_order_vol += vol
    return place_order_vol

def calc_order_vol(context, pos, side, offset, price, last_order) :
    vol = int(0)
    task_list = []
    offset_vol = (pos.volume - pos.yesterday_volume) if offset == Offset.CloseToday else pos.yesterday_volume
    offset_tag = Offset.CloseToday if offset == Offset.CloseToday else  Offset.CloseYesterday
    if(offset_vol >= context.volume_to_fill / context.steps_to_fill):
        vol = context.volume_to_fill / context.steps_to_fill
        split_order(context, vol, side, offset_tag, price, task_list, last_order)
    elif(pos.volume >= context.volume_to_fill / context.steps_to_fill):
        if(offset_vol > 0):
            vol = int(math.floor(offset_vol / float(context.MIN_VOL)) * context.MIN_VOL)
            place_order_vol = split_order(context, vol, side, offset_tag, price, task_list)
        vol = context.volume_to_fill / context.steps_to_fill - place_order_vol
        split_order(context, vol, side, Offset.Close, price, task_list, last_order)
    else:
        if(offset_vol > 0):
            vol = offset_vol
            place_order_vol = split_order(context, vol, side, offset_tag, price, task_list)
        vol = pos.volume - place_order_vol
        place_order_vol += split_order(context, vol, side, Offset.Close, price, task_list)
        vol = context.volume_to_fill / context.steps_to_fill - place_order_vol
        split_order(context, vol, side, Offset.Open, price, task_list, last_order)
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
        context.log.info(f"最后一步..................未成交订单的标的数 {pending_vol} 任务未完成数量 {context.volume_to_fill}")
    else :
        order_price = context.bid_price if context.SIDE == Side.Buy else context.ask_price
        context.log.info(f"下单计划..................任务未完成数量 {context.volume_to_fill}")
    if(context.OFFSET == Offset.Open):
        vol = context.volume_to_fill / context.steps_to_fill
        task_list = []
        split_order(context, vol, context.SIDE, Offset.Open, order_price, task_list, last_order)
        return task_list
    elif(context.OFFSET == Offset.Close or context.OFFSET == Offset.CloseYesterday):
        if(context.SIDE == Side.Buy):#Close, CloseYesterday, Side.Buy
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseYesterday, order_price, last_order)
        else:#Close, CloseYesterday, Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseYesterday, order_price, last_order)
    else: # CloseToday, Side.Buy
        if(context.SIDE == Side.Buy):
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseToday, order_price, last_order)
        else:#CloseToday,Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseToday, order_price, last_order)
    return []