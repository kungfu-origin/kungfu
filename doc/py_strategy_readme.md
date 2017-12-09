本文档主要说明策略逻辑

策略逻辑
=======

功夫交易系统是一个基于内存数据库的多线程系统，策略通过注册行情和交易获得行情信息和交易执行功能。

同时交易系统是一个多账户多策略交易系统，系统底层逻辑保证了每个策略收到自己独立的交易回报，并且系统内部集成了策略持仓管理，保障每个策略拥有独立的实时持仓。

交易系统封装了策略内部过程，只将初始化函数、持仓返回函数、行情返回函数、报单返回函数、成交返回函数以及错误返回函数暴露给用户，用户只需要在这些函数中实现自己的策略逻辑即可。

* 用户可以在初始化函数中完成自己需要的初始化操作，一般这部分操作包括，注册行情柜台和交易柜台，订阅行情，添加回调函数，设置自定义变量等。  

* 策略完成调用完初始化以后，如果注册了交易柜台且交易柜台连接正常，首先会收到一次request_id等于-1的持仓回报，如果之前策略有运行过，则传回上次策略交易结束的持仓，如果策略是首次运行，会收到一个None持仓。如果交易柜台没有启动或者意外退出了，则不会收到request_id等于-1的持仓回报。使用该项可以辅助确认策略是在交易柜台连接正常的情况下才运行。  

* 交易触发可以有行情触发，时间触发和混合触发三种方式，行情触发可以通过ticker行情和bar行情触发，这两种行情都可以通过在初始化时订阅实现

* 交易逻辑如果要通过报单交易情况来处理，可以添加在报单返回和成交返回函数中。

* 策略底层会根据成交回报更新策略持仓，可以随时通过get_pos获得


策略基本回调实现示例
==================

策略回调函数可以在安装了交易系统的环境下运行如下命令查看，以下只介绍了常用的策略回调函数。

特别要注意的是，策略实现中context是调用各种功能函数的接口，也是容纳各种自定义数据的全局变量。  

```
wingchun help function
```

## 初始化函数initialize

本函数会在策略运行之初被执行，推荐在其中完成行情柜台和交易柜台注册、订阅行情、添加回调函数、设置自定义变量等操作

示例中演示了：策略初始化时，注册了CTP行情和交易，订阅了CTP的两个合约，同时注册了时间范围内分钟线数据，并且定义了一个自定义变量。完成这些工作以后，策略会在后面收到行情时触发on_ticker, on_bar函数，用户可以根据需要在这些函数中实现自己的交易逻辑。

如果策略不需要ticker数据，可以不用实现on_ticker函数。或则策略需要多种bar数据，可以多次调用register_bar函数，不同的分钟线数据可以在on_bar函数中通过min_interval参数来区分。

SOURCE等常量定义可以使用"wingchun help constant"命令查看

```
def initialize(context):
    context.add_md(source=SOURCE.CTP)
    context.add_td(source=SOURCE.CTP)
    context.subscribe(["rb1801","rb1803"], source=SOURCE.CTP)
    context.register_bar(source=SOURCE.CTP, min_interval=1, start_time="09:00:00", end_time="22:30:00")
    context.pos_need_set = True
    context.my_data = []
```

## 持仓返回函数

本函数会在策略注册交易以后被执行一次，这次request_id=-1，pos_handler为策略持仓（如果策略之前运行过，如策略首次运行，则pos_handler=None）
之后在策略查询持仓获得账户持仓返回时会被调用  

pos_handler数据结构PosHandler可以使用"wingchun help class"命令查看  

示例中演示了，如果策略首次运行（pos_handler）则查询账户持仓，并在收到查询账户持仓以后，将账户持仓set为策略持仓。set之后将标记修改False，之后查询持仓返回则将持仓打印到debug log中。

如果要在一个账户下运行多个策略，可以使用wingchun pos 工具手动分仓，使用方法可以使用wingchun help pos查看帮助

```
def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            context.req_pos(source=SOURCE_INDEX)
    elif context.pos_need_set:
        context.set_pos(context.new_pos(source=SOURCE_INDEX), source=SOURCE_INDEX)
        context.pos_need_set = False
    else:
        context.log_debug("[POS]on_pos: {}".format(pos_handler.dump()))
```

## ticker行情函数

本函数在策略订阅了行情，收到行情的时候被执行。

market_data数据结构LFMarketDataField可以使用"wingchun help class"命令查看

```
def on_tick(context, market_data, source, rcv_time):
    context.log_info("[TICKER] (T){market_data.InstrumentID} (L){} (BP1){} (BV1){} (AP1){} (AV1){}".format(
                      market_data.InstrumentID, market_data.LastPrice, market_data.BidPrice1, market_data.BidVolume1,
                      market_data.AskPrice1, market_data.AskVolume1))
    rid = context.insert_limit_order(source=SOURCE.CTP, ticker=market_data.InstrumentID, exchange_id=context.exchange, 
                                     price=md.UpperLimitPrice, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
```

## bar行情函数

本函数在策略订阅了行情和bar数据时，收到bar的时候被执行。

bars是一个list，其数据项数据结构LFBarMarketDataField可以使用"wingchun help class"命令查看

```
def on_bar(context, bars, min_interval, source, nano):
    for key in bars:
        context.log_info("[BAR] (T){} (O){} (C){} (L){} (H){} (V){}".format(bars[key].InstrumentID, bars[key].Open, 
                         bars[key].Close, bars[key].Low, bars[key].High, bars[key].Volume))
```

## 下单回报函数

本函数在下单以后，交易所发回下单回报的时候被执行，示例中只是将收到的信息打印出来了，用户可以根据自己的需求定义处理逻辑。

rtn_order数据结构LFRtnOrderField可以使用"wingchun help class"命令查看。

order_id与执行下单函数返回的rid相同，可以用于判断返回是来自哪个下单。

策略内部处理保证了在多策略的情况下，一个策略只会收到自己下出的订单的下单回报。

```
def on_rtn_order(context, rtn_order, order_id, source, rcv_time):
    context.log_info("[RTN_ORDER] (RID){} (T){} (P){} (VTRD){} (VTOT){} (D){} (O){} (STATUS){}.format(
                     order_id, rtn_order.InstrumentID, rtn_order.VolumeTraded, rtn_order.VolumeTotal, 
                     rtn_order.Direction, rtn_order.OffsetFlag, rtn_order.OrderStatus)")
```

## 成交回报函数

本函数在下单以后，订单在交易所被撮合有成交的时候被执行，示例中只是将收到的信息打印出来了，用户可以根据自己的需求定义处理逻辑

rtn_order数据结构LFRtnOrderField可以使用"wingchun help class"命令查看

order_id与执行下单函数返回的rid相同，可以用于判断返回是来自哪个下单。

策略内部处理保证了在多策略的情况下，一个策略只会收到自己下出的订单的下单回报

```
def on_rtn_trade(context, rtn_trade, order_id, source, rcv_time):
    context.log_info("[RTN_TRADE] (RID){} (T){} (P){} (V){} (D){} (O){}".format(rtn_trade.InstrumentID,
                     order_id, rtn_trade.Price, rtn_trade.Volume, rtn_trade.Direction, rtn_trade.OffsetFlag))
```

## 除了以上函数外，还有：

on_error，错误被触发，比如订单不合法等情况的时候被执行

on_switch_day，在用户定义的换天时间点被执行，在交易需要连续运行的时候可以使用。



策略常用功能函数示例
=================

策略常用的函数功能函数通过context调用，用于实现：
* 连接行情和交易
* 各种类型下单以及查询(账户)持仓
* 策略持仓操作
* 添加不同类型回调函数
* 日志函数

具体使用方法可以使用wingchun help context查询

## 连接行情和交易

功夫系统支持多行情多交易连接，在初始化时需要使用context.add_md和context.add_td添加行情和交易。
```
context.add_md(source=SOURCE.CTP)
context.add_td(source=SOURCE.CTP)
```

## 各种类型的下单函数和查询账户持仓

下单函数包括下限价单、市价单（交易所支持的情况下，本处只是举例），fak下单，fok下单。下单函数都会返回一个整型rid，在策略中唯一标识一个订单。在下单回报函数和成交回报函数中的order_id即该标识

下单参数一般包括接口，合约，交易所，价格（限价），数量，买卖方向，开平方向

撤单函数包括接口和要撤销的订单的rid。

```
context.insert_limit_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                           price=6003.0, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
context.insert_market_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                            volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
context.insert_fok_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                         price=6003.0, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
context.insert_fak_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                         price=6003.0, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)                     
context.cancel_order(source=SOURCE.CTP, order_id=rid)
context.req_pos(source=SOURCE_INDEX)
```

## 策略持仓操作

策略持仓操作包含了获取策略持仓和设置策略持仓，使用了一个基础类为PosHandler，可以使用"wingchun help class"命令查看

set_pos函数将pos持仓数据(类型为PosHandler)设置为本策略的策略持仓，本函数可在首次运行需要设置持仓为空或者设置为账户持仓时可以使用。

get_pos函数会返回策略在该source上的持仓，返回数据类型为PosHandler，PosHandler数据格式和函数可以使用wingchun 

new_pos函数返回一个空持仓，可以用于上文中设置策略初始持仓为空的情况。

```
context.set_pos(pos_handler=pos, source=SOURCE.CTP)
context.get_pos(source=SOURCE.CTP)
context.new_pos(source=SOURCE.CTP)
```

使用不同的方式添加回调，根据回调时间计法不同和是否带context参数一共六个，以下函数意义分别为：

在1511943580122869902纳秒时间点回调call_back()函数

在当前时间十秒后，回调call_back()函数

在1511943580122869902纳秒时间点回调call_back_c(context)函数

在当前时间十秒后，回调call_back_c(context)函数

在下一个'14:50:00'，回调call_back()函数

在下一个'14:50:00'，回调call_back_c(context)函数

```
context.insert_func_at(nano=1511943580122869902, function=call_back)
context.insert_func_after(seconds=10, function=call_back_c)
context.insert_func_at_c(nano=1511943580122869902, function=call_back)
context.insert_func_after_c(seconds=10, function=call_back_c)
context.insert_func_at_next(time='14:50:00', function=call_back)
context.insert_func_at_next_c(time='14:50:00', function=call_back_c)
```


查询帮助的方法
============

在安装了kungfu开源交易系统的环境下，可以使用wingchun help来获取完整的python策略的帮助，熟练掌握wingchun help可以大大提高策略实现效率。

wingchun help包含的主要内容如下：
行情交易柜台连接方式（之一），python策略运行命令
直接读写策略持仓的方法以及实时获取策略下单、成交以及交易统计数据的方法
系统暴露给策略的功能函数接口context，实现策略逻辑的回调函数function，策略中数据结构的定义class，常量定义constant

```
wingchun md ${gateway_name} // run market data engine on gateway_name (eg: ctp)
wingchun td ${gateway_name} // run trade engine on gateway_name (eg: ctp)
wingchun strategy -n ${strategy_name} -p ${py_file_path} // run strategy in py_file_path

wingchun pos -t get/set -n ${strategy_name} -s ${source_index} // get/set strategy internal position
wingchun report -n ${strategy_name} -s ${start_time} -e ${end_time} -a // report strategy latency

wingchun help context  // context usage details
wingchun help function // functions that can be override in py_file_path
wingchun help class    // internal data structure usage.
wingchun help constant // internal constants.

wingchun help pos      // help "wingchun pos" usage
wingchun help report   // help "wingchun report" usage
```

