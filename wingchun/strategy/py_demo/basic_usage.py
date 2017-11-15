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

'''
first demo for new wingchun strategy system.
you may run this program by:
wingchun strategy -n my_test -p basic_usage.py
'''

def req_pos(context):
    print 'req_pos:', context.req_pos(source=SOURCE.CTP)

def initialize(context):
    '''
    # you may init your own parameters in context
    context.msg = 'first time try'
    context.a = 123
    '''
    print '--- running ', context.get_name(), '---'
    # add CTP market data
    context.add_md(source=SOURCE.CTP)
    # add ctp trade engine
    context.add_td(source=SOURCE.CTP)
    context.register_bar(source=SOURCE.CTP, min_interval=1, start_time='09:30:00', end_time='11:30:00') # register bar market data
    context.subscribe(tickers=['IC1711', 'rb1712'], source=SOURCE.CTP)
    context.insert_func_after_c(1, req_pos)

def on_tick(context, market_data, source, rcv_time):
    print 'id', market_data.InstrumentID, 'price', market_data.LastPrice, 'BP1', market_data.BidPrice1, 'AP1', market_data.AskPrice1

def on_bar(context, bars, min_interval, source, rcv_time):
    print 'bar:  source', source, ' rcv_time', rcv_time
    for ticker, bar in bars.items():
        print ticker, 'o', bar.Open, 'h', bar.High, 'l', bar.Low, 'c', bar.Close

def on_error(context, error_id, error_msg, request_id, source, rcv_time):
    print 'on_error:', error_id, error_msg, request_id, rcv_time, context.strategy_util.get_nano()

def on_rtn_order(context, rtn_order, order_id, source, rcv_time):
    print 'on_rtn_order:', order_id, '$', rtn_order.OrderStatus, rtn_order.OrderRef, rtn_order.InstrumentID

def on_rtn_trade(context, rtn_trade, order_id, source, rcv_time):
    print 'on_rtn_trade:', order_id, rtn_trade.OrderRef, rtn_trade.InstrumentID, rtn_trade.Volume

def on_switch_day(context, rcv_time):
    print 'on_switch_day@', rcv_time

def cancel(context):
    rid = context.cancel_order(source=SOURCE.CTP, order_id=context.order_id)
    print 'cancel', context.order_id, '->', rid

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id != -1:
        print 'POS', request_id, source, rcv_time
        for ticker in pos_handler.get_tickers():
            print '[net]', ticker, pos_handler.get_net_tot(ticker), pos_handler.get_net_yd(ticker)
            print '[long]', ticker, pos_handler.get_long_tot(ticker), pos_handler.get_long_yd(ticker)
            print '[short]', ticker, pos_handler.get_short_tot(ticker), pos_handler.get_short_yd(ticker)
        context.order_id = context.insert_limit_order(source=SOURCE.CTP, ticker='rb1801', price=3400, exchange_id=EXCHANGE.SHFE, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
        context.insert_func_after_c(1, cancel)