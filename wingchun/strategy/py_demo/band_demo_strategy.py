'''
Copyright [2017] [taurus.ai]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

import numpy as np
import pandas as pd
from collections import deque

'''
DEMO: band cross strategy
by: xcao, qliu, cjiang
'''

SOURCE_INDEX = SOURCE.CTP
M_TICKER = 'rb1801'
M_EXCHANGE = EXCHANGE.SHFE
TRADED_VOLUME_LIMIT = 500

# ----- some utility functions or classes -----
class signal():
    pass

def rolling_max(arr, period):
    return pd.rolling_max(arr, window=period)

def rolling_min(arr, period):
    return pd.rolling_min(arr, window=period)

def delay(arr, period):
    # delay period
    res = np.zeros(arr.shape) * np.nan
    res[period:] = arr[:-period]
    return res
# ----------------------------------------------

'''
initialization,
    add market data engine, add trade engine, subscription,
    and all other necessary initializations
    [necessary]
'''
def initialize(context):
    context.add_md(source=SOURCE_INDEX)
    context.add_td(source=SOURCE_INDEX)
    context.subscribe([M_TICKER], source=SOURCE_INDEX)
    # necessary initialization of internal fields.
    context.td_connected = False
    context.trade_completed = True
    context.md_num = 0
    context.traded_volume = 0
    #========= bind and initialize a signal ========
    context.signal = signal()
    context.signal.name = "sample_signal"
    context.signal.look_back = 1000
    context.signal.param1 = 200
    context.signal.param2 = 50
    context.signal.TickPrice = deque(maxlen=context.signal.look_back)
    context.signal.has_open_position = False
    context.signal.has_open_long_position = False
    context.signal.has_open_short_position = False
    context.signal.trade_size = 1

'''
callback when position is received
  if request_id > 0     this is callback of 'req_position',
  if request_id == -1   this is the ack of strategy internal position from td 
                        after 'add_td' function is called.
                        "pos_handler is None" means td has no preset position.
'''
def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1 and source == SOURCE_INDEX:
        context.td_connected = True
        context.log_info("td connected")
        if pos_handler is None:
            context.set_pos(context.new_pos(source=source), source=source)
    else:
        context.log_debug("[RSP_POS] {}".format(pos_handler.dump()))

'''
on market data,
    callback when tick is received.
    not only the tickers your strategy subscribed, 
    all market data from the source will be received. 
'''
def on_tick(context, md, source, rcv_time):
    if M_TICKER == md.InstrumentID and context.td_connected:
        context.signal.TickPrice.append(md.LastPrice)
        context.md_num += 1
        #print context.md_num,
        if context.md_num < context.signal.look_back + 2:
            return
        #============ prepare data ============
        tick_price = np.array(context.signal.TickPrice)
        upper_band = rolling_max(tick_price, period=context.signal.param1)
        upper_band = delay(upper_band, period=context.signal.param2)
        lower_band = rolling_min(tick_price, period=context.signal.param1)
        lower_band = delay(lower_band, period=context.signal.param2)
        LongEntryCondition = upper_band[-1] <= md.LastPrice
        ShortEntryCondition = lower_band[-1] >= md.LastPrice
        ExitCondition = (upper_band[-1] > md.LastPrice) and (lower_band[-1] < md.LastPrice)
        #============ generate signal ============
        if context.trade_completed:
            if LongEntryCondition and not context.signal.has_open_position:
                context.rid = context.insert_limit_order(source=SOURCE_INDEX,
                                                         ticker=md.InstrumentID,
                                                         exchange_id=M_EXCHANGE,
                                                         price = md.UpperLimitPrice,
                                                         volume=context.signal.trade_size,
                                                         direction=DIRECTION.Buy,
                                                         offset=OFFSET.Open)
                if context.rid > 0:
                    context.trade_completed = False

            if ShortEntryCondition and not context.signal.has_open_position:
                context.rid = context.insert_limit_order(source=SOURCE_INDEX,
                                                         ticker=md.InstrumentID,
                                                         exchange_id=M_EXCHANGE,
                                                         price = md.LowerLimitPrice,
                                                         volume=context.signal.trade_size,
                                                         direction=DIRECTION.Sell,
                                                         offset=OFFSET.Open)
                if context.rid > 0:
                    context.trade_completed = False

            if ExitCondition and context.signal.has_open_position:
                if context.signal.has_open_long_position:
                    context.rid = context.insert_limit_order(source=SOURCE_INDEX,
                                                             ticker=md.InstrumentID,
                                                             exchange_id=M_EXCHANGE,
                                                             price = md.LowerLimitPrice,
                                                             volume=context.signal.trade_size,
                                                             direction=DIRECTION.Sell,
                                                             offset=OFFSET.CloseToday)
                    if context.rid > 0:
                        context.trade_completed = False

                if context.signal.has_open_short_position:
                    context.rid = context.insert_limit_order(source=SOURCE_INDEX,
                                                             ticker=md.InstrumentID,
                                                             exchange_id=M_EXCHANGE,
                                                             price = md.UpperLimitPrice,
                                                             volume=context.signal.trade_size,
                                                             direction=DIRECTION.Buy,
                                                             offset=OFFSET.CloseToday)
                    if context.rid > 0:
                        context.trade_completed = False

'''
on return trade,
    callback when trade is made.
'''
def on_rtn_trade(context, rtn_trade, order_id, source, rcv_time):
    context.log_debug("[TRADE] (t){} (p){} (v){} POS:{}".format(
        rtn_trade.InstrumentID, rtn_trade.Price, rtn_trade.Volume,
        context.get_pos(source=SOURCE_INDEX).dump()))
    context.traded_volume += rtn_trade.Volume
    if context.rid == order_id :
        context.trade_completed = True
        if rtn_trade.OffsetFlag == OFFSET.Open:
            context.signal.has_open_position = True
            if rtn_trade.Direction == DIRECTION.Buy:
                context.signal.has_open_long_position = True
                context.signal.has_open_short_position = False
            elif rtn_trade.Direction == DIRECTION.Sell:
                context.signal.has_open_short_position = True
                context.signal.has_open_long_position = False
        elif rtn_trade.OffsetFlag == OFFSET.CloseToday:
            context.signal.has_open_position = False
            context.signal.has_open_short_position = False
            context.signal.has_open_long_position = False
    if context.traded_volume >= TRADED_VOLUME_LIMIT:
        context.log_info("[FINISH] traded volume limit: " + str(TRADED_VOLUME_LIMIT))
        context.stop()

'''
on error happened,
    most likely is a order rejected. related error_id and error_msg will be received.
    fail order here don't need to cancel.
'''
def on_error(context, error_id, error_msg, order_id, source, rcv_time):
    context.log_error("[ERROR] (err_id){} (err_msg){} (order_id){} (source){}".format(error_id, error_msg, order_id, source))

