# python 策略开发快速入门


参考策略的 python API，请[查看](py_strategy_api.md)

## python 策略运行过程

策略运行命令如下（其中 -n 参数后的信息为策略名，是系统识别策略的唯一标识，-p 参数后信息为策略脚本文件）

```
$ wingchun strategy -n test -p st_test.py
```

python 策略启动以后会首先执行 initialize 函数，用户可以在其中进行连接行情和交易，订阅行情，初始化自定义变量等操作。

策略通过回调函数返回信号，如 on_pos 返回持仓信息，on_tick 返回行情信息，on_bar 返回分钟行情信息，on_rtn_order 返回下单回报信息，on_rtn_trade 返回下单成交回报信息等。

用户也可以使用策略提供的接口来添加回调函数。

## 策略持仓初始化

功夫交易系统支持一个账户对应多个策略，系统为每个策略维护了一个独立的持仓。

策略在启动以后，每连接成功一个交易引擎，on_pos 函数会回调一次（此时 request_id 为 -1），返回该策略在该柜台上的持仓 pos_handler，如果策略从未被设定初始持仓，则 pos_handler 为 None。此时需要用户通过（1）策略内 set_pos 或者（2）wingchun pos 命令来为策略设定初始持仓。

策略查询账户持仓的返回也是通过on_pos回调返回，这种情况下 request_id 为请求序号（一个正整数），pos_handler 为账户持仓。

如下代码演示了策略判断策略为首次运行的情况下，查询账户持仓，并将收到的账户持仓设置为策略持仓：

```
def initialize(context):
    context.add_td(source=SOURCE.CTP)
    context.pos_set = True

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            print '-- got no position in initial, so request position --'
            context.req_pos(source=SOURCE.CTP)
            context.pos_set = False
            return
        else:
            print '-- got strategy position --'
            context.print_pos(pos_handler)
            context.stop()
    else:
        print '-- got position requested --'
        context.print_pos(pos_handler)
        if not context.pos_set:
            print "-- set strategy position --"
            context.set_pos(pos_handler, source)
        print '-- strategy position --'
        context.print_pos(context.get_pos(source=SOURCE.CTP))
        context.stop()
```

## tick行情回调中下单

可以通过 subscribe 订阅行情，订阅后 on_tick 函数会回调返回行情，行情订阅列表可以为任意合约组成的 list。

如下示例演示了订阅行情，并在 on_tick 回调中下单：

```
def initialize(context):
    # add ctp market date engine and trade engine
    context.add_md(source=SOURCE.CTP)
    context.add_td(source=SOURCE.CTP)
    context.ticker = 'rb1805'
    context.exchange_id = EXCHANGE.SHFE
    context.buy_price = -1
    context.order_rid = -1
    # subscribe
    context.subscribe(tickers=[context.ticker], source=SOURCE.CTP)

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            print '---- please set pos ----'
            context.stop()

def on_tick(context, market_data, source, rcv_time):
    if market_data.InstrumentID == context.ticker:
        context.buy_price = market_data.AskPrice1
        if context.order_rid < 0:
            context.order_rid = context.insert_limit_order(source=SOURCE.CTP,
                                                         ticker=context.ticker,
                                                         price=context.buy_price,
                                                         exchange_id=context.exchange_id,
                                                         volume=1,
                                                         direction=DIRECTION.Buy,
                                                         offset=OFFSET.Open)
```

## bar行情回调中下单

可以通过 register_bar 订阅分钟线行情，订阅后 on_bar 函数会回调返回分钟线行情，分钟线可以同时订阅多种，可以通过 on_bar 的 min_interval 进行区分。

如下示例演示了订阅分钟线行情，并在 on_bar 回调中下单：

```
def initialize(context):
    # add ctp market date engine and trade engine
    context.add_md(source=SOURCE.CTP)
    context.add_td(source=SOURCE.CTP)
    context.ticker = 'rb1805'
    context.exchange_id = EXCHANGE.SHFE
    context.buy_price = -1
    context.order_rid = -1
    # subscribe
    context.subscribe(tickers=[context.ticker], source=SOURCE.CTP)
    # register bar market data
    context.register_bar(source=SOURCE.CTP, min_interval=2, start_time='09:30:00', end_time='11:30:00')

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            print '---- please set pos ----'
            context.stop()

def on_bar(context, bars, min_interval, source, rcv_time):
    for ticker, bar in bars.items():
        if ticker == context.ticker:
            context.buy_price = bar.High
            if context.order_rid < 0:
                context.order_rid = context.insert_limit_order(source=SOURCE.CTP,
                                                         ticker=context.ticker,
                                                         price=context.buy_price,
                                                         exchange_id=context.exchange_id,
                                                         volume=1,
                                                         direction=DIRECTION.Buy,
                                                         offset=OFFSET.Open)
```

## 自定义的回调函数中下单

可以通过添加回调函数来实现在指定时间运行某个函数，添加回调有多种方法，可以指定绝对纳秒时间运行回调函数，可以指定一定秒数后运行回调函数，也可以指定在下一个时间（字符格式如 ‘09:35:00’）运行回调函数。

如下示例演示了通过添加回调函数，在获得策略持仓以后五秒执行一个下单。

```
def initialize(context):
    # add ctp market date engine and trade engine
    context.add_md(source=SOURCE.CTP)
    context.add_td(source=SOURCE.CTP)
    context.ticker = 'rb1805'
    context.exchange_id = EXCHANGE.SHFE
    context.buy_price = -1
    context.order_rid = -1
    # subscribe
    context.subscribe(tickers=[context.ticker], source=SOURCE.CTP)

def on_pos(context, pos_handler, request_id, source, rcv_time):
    if request_id == -1:
        if pos_handler is None:
            print '---- please set pos ----'
            context.stop()
        else:
            context.insert_func_after_c(5, call_insert_order)

def on_tick(context, market_data, source, rcv_time):
    if market_data.InstrumentID == context.ticker:
        context.buy_price = market_data.AskPrice1

def call_insert_order(context):
    if context.buy_price < 0:
        print '---- no order price -----'
        context.stop()
    elif context.order_rid < 0
        context.order_rid = context.insert_limit_order(source=SOURCE.CTP,
                                                 ticker=context.ticker,
                                                 price=context.buy_price,
                                                 exchange_id=context.exchange_id,
                                                 volume=1,
                                                 direction=DIRECTION.Buy,
                                                 offset=OFFSET.Open)
```


策略常用操作
=================

策略通过 context 实现如下常用操作：

* 连接行情和交易
* 查询账户持仓
* 策略持仓操作
* 下单
* 添加回调
* 输出日志

详细的函数使用方法可以[查看](py_strategy_api.md)

也可以在安装了功夫开源交易系统的环境中使用 wingchun help context 查询

## 连接行情和交易

功夫系统支持多行情多交易连接，可使用 context.add_md 和 context.add_td 连接行情和交易（需要对应柜台的行情引擎和交易引擎处于正常运行状态）。

```
context.add_md(source=SOURCE.CTP)
context.add_td(source=SOURCE.CTP)
```

## 查询账户持仓

使用如下函数可查询账户持仓，通过 on_pos 回调函数返回查询到的策略持仓。

```
context.req_pos(source=SOURCE.CTP)
```

## 策略持仓操作

策略持仓操作包含了获取策略持仓和设置策略持仓等

context.get_pos 函数会返回策略在该 source 上的持仓，返回数据类型为 PosHandler，PosHandler 数据格式和函数可以参考[python策略api文档](py_strategy_api.md)

context.set_pos 函数将持仓数据（类型为 PosHandler）设置为该策略的策略持仓，本函数可用于首次运行需要设置持仓为空仓或者需要将一个账户中的持仓分割到数个策略中时使用。

context.new_pos 函数返回一个持仓为空的结构，可以用于上文中设置策略初始持仓为空的情况。

```
context.set_pos(pos_handler=pos, source=SOURCE.CTP)
context.get_pos(source=SOURCE.CTP)
context.new_pos(source=SOURCE.CTP)
```

## 下单

下单函数包括下限价单、市价单（需要交易所支持），fak 下单，fok 下单。下单函数都会返回一个整型 rid，rid 在交易系统中唯一标识一个下单。在下单回报函数和成交回报函数中的 order_id 即该唯一标识。

下单参数一般包括下单接口，合约，交易所，价格（限价单需要），数量，买卖方向，开平方向

撤单函数包括接口和要撤销的订单的 rid。

```
rid = context.insert_limit_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                                 price=6003.0, volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)
rid = context.insert_market_order(source=SOURCE.CTP, ticker='rb1805', exchange_id=EXCHANGE.SHFE, 
                                 volume=1, direction=DIRECTION.Buy, offset=OFFSET.Open)                  
context.cancel_order(source=SOURCE.CTP, order_id=rid)
```

## 添加回调

使用不同的方式添加回调，根据回调时间计法不同和是否带 context 参数一共六个，以下函数意义分别为：

在 1511943580122869902 纳秒时间点回调 call_back() 函数

在当前时间十秒后，回调 call_back() 函数

在 1511943580122869902 纳秒时间点回调 call_back_c(context) 函数

在当前时间十秒后，回调 call_back_c(context) 函数

在下一个'14:50:00'，回调 call_back() 函数

在下一个'14:50:00'，回调 call_back_c(context) 函数

```
def call_back():
    print 'this is a basic callback function
    
def call_back_c(context):
    context.log_debug('this is a callback function with context as first parameter')

context.insert_func_at(nano=1511943580122869902, function=call_back)
context.insert_func_at_c(nano=1511943580122869902, function=call_back_c)
context.insert_func_after(seconds=10, function=call_back)
context.insert_func_after_c(seconds=10, function=call_back_c)
context.insert_func_at_next(time='14:50:00', function=call_back)
context.insert_func_at_next_c(time='14:50:00', function=call_back_c)
```
## 输出日志

功夫支持四种级别的日志，分为为 debug，info，error，fatal，下面为日志输出示例
```
context.log_debug("dubug")
context.log_info("info")
context.log_error("error")
context.log_fatal("fatal")
```