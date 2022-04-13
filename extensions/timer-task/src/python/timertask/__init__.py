from pickle import FALSE
import kungfu
import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *
import math 

yjj = kungfu.__binding__.yijinjing
class orderTask:
    def __init__(self, vol, side, offset, price):
        self.price = price
        self.vol = vol
        self.side = side
        self.offset = offset
    # def __str__(self):  # 定义打印对象时打印的字符串
    #     return " ".join(str(item) for item in (
    #         self.vol,self.side,self.offset,self.price))

def pre_start(context):
    args = {}
    context.SOURCE = ""
    context.ACCOUNT =""
    context.EXCHANGE =""
    context.TICKER = "" 
    context.BID_PRICE = 0.0
    context.ASK_PRICE = 0.0
    context.SIDE = Side.Drop
    context.OFFSET = Offset.Open
    context.VOLUME_TO_FILL = 0
    context.STEPS_TO_FILL = 0
    context.VOLUME = 0
    context.ORDERS = {} 
    context.has_quote = False
    context.min_vol = int(0)
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
    context.STEPS_TO_FILL = context.STEPS
    context.SIDE = Side(int(args["side"]))
    context.OFFSET = Offset(int(args["offset"]))
    context.VOLUME = int(args["volume"])
    context.VOLUME_TO_FILL = context.VOLUME
    context.LASTSINGULARITY = args["lastSingularity"]
    context.LASTSINGULARITYNANOSECOND = int(args["lastSingularityMilliSecond"]) * 1e6
    if(len(sourceAccountList) == 2 and len(exchangeTicker) == 5):
        context.SOURCE = sourceAccountList[0]
        context.ACCOUNT = sourceAccountList[1]
        context.EXCHANGE = exchangeTicker[0]
        context.TICKER = exchangeTicker[1]
        context.add_account(context.SOURCE, context.ACCOUNT, 100000.0)
        context.subscribe(context.SOURCE, [context.TICKER], context.EXCHANGE)
        context.log.info(f"SOURCE {context.SOURCE} context.TICKER {context.TICKER} context.EXCHANGE {context.EXCHANGE}")
    context.log.info(f"STEPS {context.STEPS} context.SIDE {context.SIDE} context.OFFSET {context.OFFSET} context.VOLUME {context.VOLUME} context.SOURCE {context.SOURCE} context.ACCOUNT {context.ACCOUNT} context.EXCHANGE {context.EXCHANGE} context.TICKER {context.TICKER} context.LASTSINGULARITY {context.LASTSINGULARITY}")

# def post_start(context):
#     context.log.info(f"post start {context.STEPS}")
#     context.log.info(f"post start {context.VOLUME}")

def type_to_minvol(argument):
    switcher = {
        InstrumentType.Stock : int(100),
        InstrumentType.Future : int(1),
    }
    return switcher.get(argument, int(1))

def on_order(context, order):
    context.log.info("[on_order 0] {}".format(order))
    if order.status == OrderStatus.Filled :
        context.log.info("******order.order_id type {}".format(type(order.order_id)))
        del context.ORDERS[order.order_id]
    elif (order.status == OrderStatus.Cancelled) or (order.status == OrderStatus.PartialFilledNotActive):
        context.log.info("[on_order 1] {}".format(context.ORDERS))
        del context.ORDERS[order.order_id]
        context.log.info("[on_order 2] {}".format(context.ORDERS))
        if (not context.ORDERS) and context.STEPS_TO_FILL > 1 :
            orders_to_insert = make_plan(context, False)
            context.log.info("------------orders_to_insert----------------")
            for item in orders_to_insert:
                context.log.info("now {} insert_order_task orders_to_insert {}".format(context.now(), item.__dict__))
            make_order(context, orders_to_insert)
            context.STEPS_TO_FILL = context.STEPS_TO_FILL - 1
    elif (order.status == OrderStatus.Error) :
        del context.ORDERS[order.order_id] 

def on_trade(context, trade):
    context.log.info("[on_trade] {}".format(trade))
    context.ORDERS[trade.order_id] -= trade.volume
    context.VOLUME_TO_FILL = context.VOLUME_TO_FILL - trade.volume

def on_quote(context, quote):
    context.BID_PRICE = quote.bid_price[0]
    context.ASK_PRICE = quote.ask_price[0]
    context.UPPER_LIMIT_PRICE = quote.upper_limit_price
    context.LOWER_LIMIT_PRICE = quote.lower_limit_price
    if context.min_vol == 0 :
        context.log.info("instrument_id {} bid {} ask {} up {} down {}".format(quote.instrument_id, str(quote.bid_price)[1:-1], str(quote.ask_price)[1:-1], quote.upper_limit_price, quote.lower_limit_price))
        context.min_vol = type_to_minvol(quote.instrument_type)
        context.log.info("quote.instrument_type {} context.min_vol {}".format(quote.instrument_type, context.min_vol))

    if not context.has_quote :
        startTimeInNano = int(context.now()  + 3 * 1e9)
        finishTimeInNano = int(startTimeInNano  + 2 * 60 * 1e9)
        context.log.info("startTimeInNano {}".format(startTimeInNano))
        context.log.info("finishTimeInNano {}".format(finishTimeInNano))
        context.log.info("startTimeInNano {}".format(kft.strftime(int(startTimeInNano))))
        context.log.info("finishTimeInNano {}".format(kft.strftime(int(finishTimeInNano))))
        interval = int((finishTimeInNano - startTimeInNano) / (context.STEPS - 1))
        context.log.info(f"interval {interval}")
        for i in range(context.STEPS):
            if i == context.STEPS - 1:
                last_order_time = int(startTimeInNano + interval * i - context.LASTSINGULARITYNANOSECOND) if context.LASTSINGULARITY == 'true' else int(startTimeInNano + interval * i)
                last_order_cancel_time = int(startTimeInNano + interval * i - (interval + context.LASTSINGULARITYNANOSECOND) / 2) if context.LASTSINGULARITY == 'true' else int(startTimeInNano + interval * i - interval / 2)
                context.add_timer(last_order_cancel_time, lambda ctx, event: cancel_all_orders(ctx))
                context.add_timer(last_order_time, lambda ctx, event: insert_order_task(ctx, True))
            else:
                context.add_timer(int(startTimeInNano + interval * i), lambda ctx, event: insert_order_task(ctx, False))
        context.has_quote = True

def insert_order_task(context, last_order):
    if (not last_order) and context.ORDERS:
        cancel_all_orders(context)
    else :
        orders_to_insert = make_plan(context, last_order)
        context.log.info("------------orders_to_insert----------------")
        for item in orders_to_insert:
            context.log.info("now {} insert_order_task orders_to_insert {}".format(context.now(), item.__dict__))
        make_order(context, orders_to_insert)
        context.STEPS_TO_FILL = context.STEPS_TO_FILL - 1

def cancel_all_orders(context):
    context.log.info("[cancel_all_orders] {}".format(context.ORDERS))
    for item in context.ORDERS.keys():
        context.log.info(f"canecl order id {item}")
        context.cancel_order(item)
    
def make_order(context, orders):
    for item in orders:
        order_id = context.insert_order(context.TICKER, context.EXCHANGE, context.ACCOUNT, item.price, item.vol, PriceType.Limit, item.side, item.offset)
        context.ORDERS[order_id] = item.vol
    context.log.info("[make_order] context.ORDERS {}".format(context.ORDERS))

def calc_order_vol(context, pos, side, offset, price) :
    vol = int(0)
    taskList = []
    offsetVol = (pos.volume - pos.yesterday_volume) if offset == Offset.CloseToday else pos.yesterday_volume
    offsetTag = Offset.CloseToday if offset == Offset.CloseToday else  Offset.CloseYesterday
    if(offsetVol >= context.VOLUME_TO_FILL/context.STEPS_TO_FILL):
        vol = int(math.floor(context.VOLUME_TO_FILL/context.STEPS_TO_FILL/float(context.min_vol)) * context.min_vol)
        if vol > 0 :
            taskList.append(orderTask(vol, side, offsetTag, price))
    elif(pos.volume >= context.VOLUME_TO_FILL/context.STEPS_TO_FILL):
        if(offsetVol > 0):
            vol = int(math.floor(offsetVol/float(context.min_vol)) * context.min_vol)
            if vol > 0 :
                taskList.append(orderTask(vol, side, offsetTag, price))
        vol = int(math.floor((context.VOLUME_TO_FILL/context.STEPS_TO_FILL - vol)/float(context.min_vol)) * context.min_vol)
        if vol > 0 :
            taskList.append(orderTask(vol, side, Offset.Close, price))
    else:
        placeOrderVol = int(0)
        if(offsetVol > 0):
            vol = int(math.floor(offsetVol/float(context.min_vol)) * context.min_vol)
            placeOrderVol = placeOrderVol + vol
            if vol > 0 :
                taskList.append(orderTask(vol, side, offsetTag, price))
        vol = int(math.floor((pos.volume - vol)/float(context.min_vol)) * context.min_vol)
        placeOrderVol = placeOrderVol + vol
        if vol > 0 :
            taskList.append(orderTask(vol, side, Offset.Close, price))
        vol = int(math.floor((context.VOLUME_TO_FILL/context.STEPS_TO_FILL - placeOrderVol)/float(context.min_vol)) * context.min_vol)
        if vol > 0 :
            taskList.append(orderTask(vol, side, Offset.Open, price))
    return taskList

def make_plan(context, last_order):
    book = context.get_account_book(context.SOURCE, context.ACCOUNT)
    vol = int(0)
    if last_order :
        pending_vol = int(0)
        for order_vol in context.ORDERS.values():
            pending_vol += order_vol
        orderPrice = context.UPPER_LIMIT_PRICE if context.SIDE==Side.Buy else context.LOWER_LIMIT_PRICE
        context.VOLUME_TO_FILL -=  pending_vol
    else :
        orderPrice = context.BID_PRICE if context.SIDE==Side.Buy else context.ASK_PRICE
    if(context.OFFSET == Offset.Open):
        vol = int(math.floor(context.VOLUME_TO_FILL/context.STEPS_TO_FILL/float(context.min_vol)) * context.min_vol)
        context.log.info(f"make_plan..................vol {vol}")
        if vol > 0 :
            return [orderTask(vol, context.SIDE, Offset.Open, orderPrice)]
    elif(context.OFFSET == Offset.Close or context.OFFSET == Offset.CloseYesterday):
        if(context.SIDE==Side.Buy):#Close, CloseYesterday, Side.Buy
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseYesterday, orderPrice)
        else:#Close, CloseYesterday, Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseYesterday, orderPrice)
    else: # CloseToday, Side.Buy
        if(context.SIDE==Side.Buy):
            for key in book.short_positions:
                pos = book.short_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Buy, Offset.CloseToday, orderPrice)
        else:#CloseToday,Side.Sell
            for key in book.long_positions:
                pos = book.long_positions[key]
                if(pos.instrument_id == context.TICKER):
                    return calc_order_vol(context, pos, Side.Sell, Offset.CloseToday, orderPrice)
    return []