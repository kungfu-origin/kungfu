import json
import kungfu
from kungfu.wingchun.constants import *
import math
import time
from datetime import datetime

lf = kungfu.__binding__.longfist
wc = kungfu.__binding__.wingchun
yjj = kungfu.__binding__.yijinjing


class OrderTask(object):
    def __init__(self, order):
        self.order_id = order.order_id
        self.order = order
        self.left_ratio = order.volume_left / order.volume


class TWAP(object):
    def __init__(self, init_param):
        self.last_price = None
        self.lower_limit_price = None
        self.upper_limit_price = None
        self.realtime_quote = None
        self.ask_price = None
        self.bid_price = None
        self.min_volume = 1
        self.map_order_tasks = {}  # 统计未完成比例

        sourceAccountList = init_param["accountId"].split('_')
        self.source_id = sourceAccountList[0]
        self.account_id = sourceAccountList[1]
        self.marketSource = init_param["marketSource"]
        exchangeTicker = init_param["ticker"].split('_')
        self.exchange_id = exchangeTicker[0]
        self.instrument_id = exchangeTicker[1]
        self.side = Side(int(init_param["side"]))
        self.offset = Offset(int(init_param["offset"]))
        self.priceMode = int(init_param["priceMode"])
        self.priceType = PriceType(int(init_param["priceType"]))
        self.volume = int(init_param["volume"])
        self.startTime = init_param["startTime"]
        self.interval = int(init_param["interval"])
        self.maxLotByStep = int(init_param["maxLotByStep"])

        self.step_count = 0  # 已完成步数
        self.traded_volume = 0  # 实际已完成的成交量
        self.total_volume = 0  # 已下单数量
        self.map_orders = {}  # order_map {order_id : volume_left} on_trade filled或者on_order cancelled时删除, 下单时添加
        self.total_left = 0  # 当前时刻之前所有下单未完成的数量
        self.start_time_in_nano = str_to_nanotime(self.startTime)
        self.interval_in_nano = int(self.interval) * 10 ** 6
        self.on_start = False

    def on_quote(self, quote, context):
        self.bid_price = quote.bid_price[0]
        self.ask_price = quote.ask_price[0]
        self.last_price = quote.last_price
        self.realtime_quote = quote
        self.upper_limit_price = quote.upper_limit_price
        self.lower_limit_price = quote.lower_limit_price
        if not self.on_start:
            self.add_timer_task(context)
            self.on_start = True
            self.min_volume = type_to_minvol(quote.instrument_type)
            self.maxLotByStep -= self.maxLotByStep % self.min_volume  # 保证最大下单量为min_volume的整数倍
            self.volume -= self.volume % self.min_volume  # 把每次原计划下单量改为min_volume的整数倍
            assert self.volume >= self.min_volume
            context.log.info(
                f"""
                self.min_volume {self.min_volume}
                self.volume {self.volume}
                self.maxLotByStep {self.maxLotByStep}                         
                """)

    def get_price(self, context):
        # 最新价
        if self.priceMode == 2:
            return self.last_price

        # 对手价一档
        if self.priceMode == 0:
            if self.side == Side.Buy:
                return self.ask_price
            if self.side == Side.Sell:
                return self.bid_price

        # 同方向一档
        if self.priceMode == 1:
            if self.side == Side.Sell:
                return self.ask_price
            if self.side == Side.Buy:
                return self.bid_price

        context.log.error("Invalidate Condition When Getting Price!")

    def validate_volume(self, try_volume):
        remainder = try_volume % self.min_volume
        delta_left = 0
        # 不等于0则不能以这个数值下单, 需要减去余数, 或者进位补齐, 才能下单
        if remainder != 0:
            try_volume -= remainder  # 减去余数部分
            delta_left = remainder  # 影响到total_left的增量

        return try_volume, delta_left

    def get_volume(self, context):
        context.log.warn(f"累计已下单但未完成的数量: {context.twap.total_left}")
        try_volume = self.total_left + self.volume  # 累计未完成单子数量 + 每切片应下单数量
        context.twap.total_left = 0  # 把累计的全部加到本次下单，逻辑上就清空了累积额，下面计算下单数量合法性时会把无法下单的部分再补回来
        # context.log.info(f"self.total_left : {self.total_left} , try_volume 0 : {try_volume}")
        # 校验下单数量是否符合该金融品种的最小下单数额的整数倍
        try_volume, delta_left = self.validate_volume(try_volume)
        self.total_left += delta_left
        # context.log.info(f"try_volume : {try_volume}, delta_left : {delta_left}, self.total_left : {self.total_left}")

        # 下单数量超过限制, 超出限制部分累放到下次下单, maxLotByStep会被校验成符合该金融品种的最小下单数额的整数倍
        if try_volume > self.maxLotByStep:
            try_volume = self.maxLotByStep
            self.total_left += try_volume - self.maxLotByStep
        # else:
        #     # 未超出限制, 则累计清零
        #     self.total_left = 0

        self.total_volume += try_volume
        context.log.info(
            f"try_volume : {try_volume}, delta_left : {delta_left}, self.total_left : {self.total_left}, self.total_volume : {self.total_volume}")
        return try_volume

    def add_timer_task(self, context):
        self.start_time_in_nano = max(self.start_time_in_nano, context.now())  # 如果设置的启动时间早于现在, 则视为从现在开始启动
        context.add_timer(self.start_time_in_nano, lambda ctx, event: add_timer_task_insert(ctx))

    def cancel_orders(self, context):
        for k, v in self.map_orders.items():
            context.cancel_order(k)


def type_to_minvol(argument):
    switcher = {
        InstrumentType.Stock: int(100),
        InstrumentType.Future: int(1),
        InstrumentType.Bond: int(1),
        InstrumentType.StockOption: int(1),
        InstrumentType.Fund: int(1),
        InstrumentType.TechStock: int(200),
        InstrumentType.Index: int(1),
    }
    return switcher.get(argument, int(1))


def add_timer_task_insert(context):
    next_timer_in_nano = context.now() + context.twap.interval_in_nano
    context.add_timer(next_timer_in_nano, lambda ctx, event: insert_order_task(ctx))


def insert_order_task(context):
    print_datatime(context, "execute insert_order_task", context.now())
    context.twap.cancel_orders(context)

    order_price = context.twap.get_price(context)
    order_volume = context.twap.get_volume(context)
    order_price_type = context.twap.priceType
    context.log.warn(
        f"""
        当前为第 {context.twap.step_count + 1} 次下单, 标的 {context.twap.instrument_id}, 交易所 {context.twap.exchange_id},
        柜台 : {context.twap.source_id} , 账户 : {context.twap.account_id} , 买卖 : {context.twap.side} , 开平 : {context.twap.offset}
        选价方式 : {context.twap.priceMode} , 下单方式 : {context.twap.priceType}
        本次下单价格 : {order_price} ,  本次下单数量 :{order_volume}
        累计已下单但未完成需要补单的数量 :{context.twap.total_left}
        累积成交数量 {context.twap.traded_volume}
        """)

    order_id = context.insert_order(context.twap.instrument_id, context.twap.exchange_id, context.twap.account_id,
                                    order_price, order_volume, order_price_type, context.twap.side,
                                    context.twap.offset)
    context.twap.map_orders[order_id] = order_volume
    context.twap.step_count += 1  # 实际下单完成后才算完成了一步
    context.twap.total_left += order_volume
    context.log.warn(f"累计已下单但未完成的数量: {context.twap.total_left}")

    strategy_state = lf.types.StrategyStateUpdate
    strategy_state.state = lf.enums.StrategyState.Nomal
    strategy_state.info_a = json.dump({"left_ratio": context.twap.total_left / context.twap.total_insert})
    strategy_state.info_b = "random message"
    context.update_strategy_state(strategy_state)

    add_timer_task_insert(context)


def pre_start(context):
    # context.hold_book()
    # context.hold_positions()
    context.log.info("arguments {}".format(context.arguments))
    context.twap = TWAP(json.loads(context.arguments))
    context.add_account(context.twap.source_id, context.twap.account_id, 100000.0)
    context.subscribe(context.twap.marketSource, [context.twap.instrument_id], context.twap.exchange_id)
    context.log.info(
        f"""
            SOURCE {context.twap.source_id} 
            ACCOUNT {context.twap.account_id} 
            MARKETDATA SOURCE {context.twap.marketSource}
            EXCHANGE {context.twap.exchange_id} 
            TICKER {context.twap.instrument_id}        
            SIDE {context.twap.side} 
            OFFSET {context.twap.offset} 
            VOLUME PER STEP {context.twap.volume} 
        """)


def str_to_nanotime(tm):
    if (tm.isdigit()):  # in milliseconds
        return int(tm) * 10 ** 6
    else:
        year_month_day = time.strftime("%Y-%m-%d", time.localtime())
        ymdhms = year_month_day + ' ' + tm.split(' ')[1]
        timeArray = time.strptime(ymdhms, "%Y-%m-%d %H:%M:%S")
        nano = int(time.mktime(timeArray) * 10 ** 9)
        return nano


def type_to_minvol(argument):
    switcher = {
        InstrumentType.Stock: int(100),
        InstrumentType.Future: int(1),
        InstrumentType.Bond: int(1),
        InstrumentType.StockOption: int(1),
        InstrumentType.Fund: int(1),
        InstrumentType.TechStock: int(200),
        InstrumentType.Index: int(1),
    }
    return switcher.get(argument, int(1))


def on_order(context, order):
    context.log.info("[on_order] {}".format(order))
    # context.log.info(f"context.twap.map_orders {context.twap.map_orders}")
    context.twap.map_order_tasks[order.order_id] = OrderTask(order)
    if order.status == OrderStatus.Filled:
        del context.twap.map_orders[order.order_id]
    elif (order.status == OrderStatus.Cancelled) or (order.status == OrderStatus.PartialFilledNotActive):
        del context.twap.map_orders[order.order_id]
    elif order.status == OrderStatus.Error:
        context.log.error(f"Order Error! : error_id : {order.error_id}, error_msg : {order.error_msg}")
        del context.twap.map_orders[order.order_id]


def on_trade(context, trade):
    context.log.info("[on_trade] {}".format(trade))
    if trade.order_id in context.twap.map_orders:
        context.twap.map_orders[trade.order_id] -= trade.volume
    context.twap.total_left -= trade.volume
    context.twap.traded_volume += trade.volume
    context.log.warn(f"累计已下单但未完成的数量: {context.twap.total_left}")


def print_datatime(context, info, nano):
    date_time_for_nano = datetime.fromtimestamp(nano / (10 ** 9))
    time_str = date_time_for_nano.strftime("%Y-%m-%d %H:%M:%S.%f")
    context.log.info("{} nano {} datetime {}".format(info, nano, time_str))


def on_quote(context, quote):
    context.log.info(f"[on_quote] : {quote}")
    context.twap.on_quote(quote, context)
