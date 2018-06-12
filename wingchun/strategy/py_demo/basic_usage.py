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
    print('req_pos:{}'.format(context.req_pos(source=SOURCE.CTP)))


def initialize(context):
    '''
    # you may init your own parameters in context
    context.msg = 'first time try'
    context.a = 123
    '''
    print('--- running ' + context.get_name() + '---')
    # add CTP market data
    context.add_md(source=SOURCE.CTP)
    # add ctp trade engine
    context.add_td(source=SOURCE.CTP)
    context.register_bar(source=SOURCE.CTP, min_interval=1, start_time='09:30:00',
                         end_time='11:30:00')  # register bar market data
    context.subscribe(tickers=['rb1809'], source=SOURCE.CTP)
    context.insert_func_after_c(1, req_pos)


def on_tick(context, market_data, source, rcv_time):
    print('[ON_TICK] InstrumentID {} price {} BP1 {} AP1 {}'.format(market_data.InstrumentID, market_data.LastPrice,
                                                                    market_data.BidPrice1, market_data.AskPrice1))
    if context.order_id:
        return
    price = market_data.BidPrice1
    print('[ON_TICK]--------------------- insert order PRICE {} ---------------------------'.format(price))
    context.order_id = context.insert_limit_order(source=SOURCE.CTP, ticker=market_data.InstrumentID,
                                                  price=price,
                                                  exchange_id=EXCHANGE.SHFE, volume=1, direction=DIRECTION.Buy,
                                                  offset=OFFSET.Open)
    context.insert_func_after_c(20, cancel)


def cancel(context):
    if not context.order_id:
        return
    rid = context.cancel_order(source=SOURCE.CTP, order_id=context.order_id)
    print('[CANCEL] ORDER_ID {} -> RID {}'.format(context.order_id, rid))


def on_bar(context, bars, min_interval, source, rcv_time):
    print('[ON_BAR]:  source {}  rcv_time {}'.format(source, rcv_time))
    for ticker, bar in bars.items():
        print('[ON_BAR] ticker {} o {} h {} l {} c {}'.format(ticker, bar.Open, bar.High, bar.Low, bar.Close))


def on_error(context, error_id, error_msg, request_id, source, rcv_time):
    print('[ON_ERROR]: ERROR_ID {}  ERROR_MSG {} REQUEST_ID {}  RCV_TIME {} NANO {}'.format(error_id, error_msg,
                                                                                            request_id, rcv_time,
                                                                                            context.strategy_util.get_nano()))


def on_rtn_order(context, rtn_order, order_id, source, rcv_time):
    print('[ON_RTN_ORDER]:(order_id){}, (OrderStatus){}, (OrderRef){}, (InstrumentID){}'.format(order_id,
                                                                                                rtn_order.OrderStatus,
                                                                                                rtn_order.OrderRef,
                                                                                                rtn_order.InstrumentID))
    if order_id == context.order_id and rtn_order.OrderStatus == ORDER_STATUS.Canceled:
        print('[ON_RTN_ORDER]:-----------------------------------order cancelled------------------------------')
        context.order_id = None


def on_rtn_trade(context, rtn_trade, order_id, source, rcv_time):
    print('[ON_RTN_TRADE]:ORDER_ID {} ORDER_REF{} TICKER {} VOLUME {}'.format(order_id, rtn_trade.OrderRef,
                                                                              rtn_trade.InstrumentID, rtn_trade.Volume))
    if order_id == context.order_id:
        context.order_id = None
        print('[ON_RTN_TRADE]===========================================order traded, finish!========================')
        context.stop()


def on_switch_day(context, rcv_time):
    print('[ON_SWITCH_DAY] RCV_TIME {}'.format(rcv_time))


def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id != -1:
        print('[ON_POS] REQUEST_ID {} SOURCE {} RCV_TIME {}'.format(request_id, source, rcv_time))
        context.set_pos(pos_handler, SOURCE.CTP)
        for ticker in pos_handler.get_tickers():
            print('[ON_POS] TICKER {} NET_TOT {} NET_YD {}'.format(ticker, pos_handler.get_net_tot(ticker),
                                                                   pos_handler.get_net_yd(ticker)))
            print('[ON_POS] TICKER {} LONG_TOT {} LONG_YD {}'.format(ticker, pos_handler.get_long_tot(ticker),
                                                                     pos_handler.get_long_yd(ticker)))
            print('[ON_POS]TICKER {} SHORT_TOT {} SHORT_YD {}'.format(ticker, pos_handler.get_short_tot(ticker),
                                                                      pos_handler.get_short_yd(ticker)))
