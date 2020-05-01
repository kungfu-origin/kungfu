'''
@Author: qliu
@Date: 2019-08-19 14:57:08
@LastEditors: qliu
@LastEditTime: 2019-08-19 15:49:00
@Description: file content
'''
import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

SOURCE = Source.CTP
# ACCOUNT = "089270"
ACCOUNT = "115796"
# tickers = ["AG2005", "AG2006", "ZN2006", "ZN2009"]
# tickers = ["rb1911","rb1912"]
# tickers = ["rb1911","c1911","MA1911","rb1912"]
# tickers = ["603993","603986","603833","603799","603288","603260","603259"]
# tickers = ["603993","603986","603833","603799","603288","603260","603259"]
VOLUME = 2
EXCHANGE = Exchange.SHFE


# 启动前回调，添加交易账户，订阅行情，策略初始化计算等
def pre_start(context):
    context.add_account(Source.CTP, ACCOUNT, 100000.0)
    context.subscribe(Source.CTP, ["rb2005"], Exchange.SHFE)
    # context.subscribe(Source.BAR, ["rb2005", "rb2010"], Exchange.SHFE)
    # context.subscribe(Source.CTP, ["BB2005", "BB2006", "EG2005", "EG2006"], Exchange.DCE)


# 启动准备工作完成后回调，策略只能在本函数回调以后才能进行获取持仓和报单
def post_start(context):
    context.log.warning("post_start")
    for key in context.book.long_positions:
        context.log.info(f'key {key}')
        pos = context.book.long_positions[key]
        context.log.info(f'close long position {pos}')
        side = Side.Sell if pos.direction == Direction.Long else Side.Buy
        context.insert_order(pos.instrument_id, pos.exchange_id, pos.account_id, pos.avg_open_price, pos.volume + 10, PriceType.Limit, side, Offset.Close)
        context.log.info('closed')
    # context.add_timer(context.now() + 1*1000000000, log_book)


def log_book(context, event):
    context.log.info("[avail]{}".format(context.book.asset.avail))
    context.log.info("[acc_avail]{}".format(context.get_account_book(SOURCE, ACCOUNT).avail))
    book = context.book
    asset = book.asset
    context.log.info("[strategy capital] (avail){} (margin){} (market_value){} (initial_equity){} (dynamic_equity){} (static_equity){} \
        (realized_pnl){} (unrealized_pnl){}".format(asset.avail, asset.margin, asset.market_value, asset.initial_equity,
                                                    asset.dynamic_equity, asset.static_equity, asset.realized_pnl,
                                                    asset.unrealized_pnl))
    book = context.get_account_book(SOURCE, ACCOUNT)
    asset = book.asset
    context.log.info("[account capital] (avail){} (margin){} (market_value){} (initial_equity){} (dynamic_equity){} (static_equity){} \
        (realized_pnl){} (unrealized_pnl){}".format(asset.avail, asset.margin, asset.market_value, asset.initial_equity,
                                                    asset.dynamic_equity, asset.static_equity, asset.realized_pnl,
                                                    asset.unrealized_pnl))
    for pos in context.get_account_book(SOURCE, ACCOUNT).positions:
        log_stock_pos(context, pos)
        # log_future_pos(context, pos)
    for pos in context.book.positions:
        log_stock_pos(context, pos)
        # log_future_pos(context, pos)


# 收到快照行情时回调，行情信息通过quote对象获取 
def on_quote(context, quote):
    # context.log.info(f"insert order for {quote.instrument_id}")
    # order_id = context.insert_order(quote.instrument_id, EXCHANGE, ACCOUNT, quote.last_price, VOLUME,
    #                                 PriceType.Limit, Side.Buy, Offset.Open)
    # if order_id > 0:
    #     # context.log.info("[order] (rid){} (ticker){}".format(order_id, quote.instrument_id))
    #     # 通过添加时间回调，在5秒以后撤单
    #     context.add_timer(context.now() + 5 * 1000000000, lambda ctx, event: cancel_order(ctx, order_id))
    # else:
    #     context.log.info(f"insert order failed {order_id}")
    pass


# 收到k线行情时回调，行情信息通过bar对象获取
def on_bar(context, bar):
    context.log.info("[on_bar] {}".format(bar))


# 收到订单状态回报时回调
def on_order(context, order):
    # context.log.info("[on_order] {}".format(order))
    pass


# 收到成交信息回报时回调
def on_trade(context, trade):
    context.log.info("[on_trade] {}".format(trade))
    pass


# 策略释放资源前回调，仍然可以获取持仓和报单
def pre_stop(context):
    context.log.info("[befor strategy stop]")


# 策略释放资源后回调
def post_stop(context):
    context.log.info("[befor process stop]")


## 自定义函数

# 自定义期货持仓数据日志输出函数
def log_future_pos(context, pos):
    context.log.info("(instrument_id){} (instrument_type){} (exchange_id){}(direction){} (volume){} (yesterday_volume){} \
        (last_price){} (settlement_price){} (pre_settlement_price){} (avg_open_price){} (position_cost_price){} (margin){}\
            (position_pnl){} (realized_pnl){} (unrealized_pnl){} ".format(pos.instrument_id, pos.instrument_type,
                                                                          pos.exchange_id,
                                                                          pos.direction, pos.volume,
                                                                          pos.yesterday_volume, pos.last_price,
                                                                          pos.settlement_price,
                                                                          pos.pre_settlement_price,
                                                                          pos.avg_open_price, pos.position_cost_price,
                                                                          pos.margin, pos.position_pnl,
                                                                          pos.realized_pnl, pos.unrealized_pnl))


# 自定义股票持仓数据日志输出函数
def log_stock_pos(context, pos):
    context.log.info("(instrument_id){} (instrument_type){} (exchange_id){} (direction){} (volume){} (yesterday_volume){} \
        (last_price){} (pre_close_price){} (close_price){} (avg_open_price){} (position_cost_price){} (realized_pnl){}\
            (unrealized_pnl){} ".format(pos.instrument_id, pos.instrument_type, pos.exchange_id, pos.direction,
                                        pos.volume, pos.yesterday_volume, pos.last_price, pos.pre_close_price,
                                        pos.close_price, pos.avg_open_price,
                                        pos.position_cost_price, pos.realized_pnl, pos.unrealized_pnl))


# 自定义撤单回调函数
def cancel_order(context, order_id):
    action_id = context.cancel_order(order_id)
    if action_id > 0:
        context.log.info("[cancel order] (action_id){} (rid){} ".format(action_id, order_id))
