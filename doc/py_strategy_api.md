函数列表
=======

* 回调函数
  * [initialize](#initialize) &emsp;&emsp;&emsp;&emsp; 初始化函数 
  * [on_tick](#on_tick) &emsp;&emsp;&emsp;&emsp; tick 行情回调函数
  * [on_bar](#on_bar) &emsp;&emsp;&emsp;&emsp; 分钟线行情回调函数
  * [on_pos](#on_pos) &emsp;&emsp;&emsp;&emsp; 持仓返回回调函数 
  * [on_rtn_order](#on_rtn_order)	 &emsp;&emsp;&emsp;&emsp; 下单回报回调函数 
  * [on_rtn_trade](#on_rtn_trade)	 &emsp;&emsp;&emsp;&emsp; 成交回报回调函数 
  * [on_error](#on_error) &emsp;&emsp;&emsp;&emsp;错误消息回调函数 
* 功能函数
  * [context.add_md](#context.add_md) &emsp;&emsp;&emsp;&emsp; 连接行情
  * [context.add_td](#context.add_td) &emsp;&emsp;&emsp;&emsp; 连接交易
  * [context.req_pos](#context.req_pos) &emsp;&emsp;&emsp;&emsp; 查询持仓
  * [context.new_pos](#context.new_pos) &emsp;&emsp;&emsp;&emsp; 生成空持仓
  * [context.set_pos](#context.set_pos) &emsp;&emsp;&emsp;&emsp; 设置策略持仓
  * [context.get_pos](#context.get_pos) &emsp;&emsp;&emsp;&emsp; 获取策略持仓
  * [context.print_pos](#context.print_pos) &emsp;&emsp;&emsp;&emsp; 打印持仓
  * [context.subscribe](#context.subscribe) &emsp;&emsp;&emsp;&emsp; 订阅行情
  * [context.register_bar](#context.register_bar) &emsp;&emsp;&emsp;&emsp; 订阅行情分钟线数据
  * [context.stop](#context.stop)	&emsp;&emsp;&emsp;&emsp; 停止策略
  * [context.get_nano](#context.get_nano) &emsp;&emsp;&emsp;&emsp; 获取系统当前纳秒时间
  * [context.get_name](#context.get_name) &emsp;&emsp;&emsp;&emsp; 获取策略名
  * [context.parse_time](#context.parse_time) &emsp;&emsp;&emsp;&emsp; 时间字符串转纳秒时间
  * [context.parse_nano](#context.parse_nano) &emsp;&emsp;&emsp;&emsp; 纳秒时间转时间字符串
  * [context.insert_func_at](#context.insert_func_at) &emsp;&emsp;&emsp;&emsp; 在指定纳秒时间添加函数回调（无参数）
  * [context.insert_func_after](#context.insert_func_after) &emsp;&emsp;&emsp;&emsp; 在当前时间指定秒后添加回调（无参数）
  * [context.insert_func_at_c](#context.insert_func_at_c) &emsp;&emsp;&emsp;&emsp; 在指定纳秒时间添加函数回调（context为参数）
  * [context.insert_func_after_c](#context.insert_func_after_c) &emsp;&emsp;&emsp;&emsp; 在当前时间后指定秒添加回调（context为参数）
  * [context.insert_market_order](#context.insert_market_order) &emsp;&emsp;&emsp;&emsp; 下市价单
  * [context.insert_limit_order](#context.insert_limit_order) &emsp;&emsp;&emsp;&emsp; 下限价单
  * [context.insert_fok_order](#context.insert_fok_order) &emsp;&emsp;&emsp;&emsp; 下fok订单
  * [context.insert_fak_order](#context.insert_fak_order) &emsp;&emsp;&emsp;&emsp; 下fak订单
  * [context.cancel_order](#context.cancel_order) &emsp;&emsp;&emsp;&emsp; 撤销订单
  * [context.log_debug](#context.log_debug) &emsp;&emsp;&emsp;&emsp; 输入debug日志
  * [context.log_info](#context.log_info) &emsp;&emsp;&emsp;&emsp; 输出info日志
  * [context.log_error](#context.log_error) &emsp;&emsp;&emsp;&emsp; 输出error日志
  * [context.log_fatal](#context.log_fatal) &emsp;&emsp;&emsp;&emsp; 输出fatal日志
* 持仓成员函数
  * [PosHandler.add_pos](#PosHandler.add_pos) &emsp;&emsp;&emsp;&emsp; 添加持仓
  * [PosHandler.set_pos](#PosHandler.set_pos) &emsp;&emsp;&emsp;&emsp; 设置持仓
  * [PosHandler.dump](#PosHandler.dump) &emsp;&emsp;&emsp;&emsp; 持仓转为 json 格式的string
  * [PosHandler.load](#PosHandler.load) &emsp;&emsp;&emsp;&emsp; 载入 json 格式的持仓字符串持仓
  * [PosHandler.get_tickers](#PosHandler.get_tickers) &emsp;&emsp;&emsp;&emsp; 获取有持仓的合约列表 
  * [PosHandler.get_net_tot](#PosHandler.get_net_tot) &emsp;&emsp;&emsp;&emsp; 获取指定股票合约的总持仓
  * [PosHandler.get_net_yd](#PosHandler.get_net_yd) &emsp;&emsp;&emsp;&emsp; 获取指定股票合约的昨仓
  * [PosHandler.get_long_tot](#PosHandler.get_long_tot) &emsp;&emsp;&emsp;&emsp; 获取指定合约的总多仓
  * [PosHandler.get_long_yd](#PosHandler.get_long_yd) &emsp;&emsp;&emsp;&emsp; 获取指定合约的昨日多仓
  * [PosHandler.get_short_tot](#PosHandler.get_short_tot) &emsp;&emsp;&emsp;&emsp; 获取指定合约的总空仓
  * [PosHandler.get_short_yd](#PosHandler.get_short_yd) &emsp;&emsp;&emsp;&emsp; 获取指定合约的昨日空仓

数据结构
===============
* [LFMarketDataField](#LFMarketDataField) &emsp;&emsp;&emsp;&emsp; 行情数据结构
* [LFBarMarketDataField](#LFBarMarketDataField) &emsp;&emsp;&emsp;&emsp; 分钟线行情数据结构
* [LFRtnOrderField](#LFRtnOrderField) &emsp;&emsp;&emsp;&emsp; 下单回报数据结构
* [LFRtnTradeField](#LFRtnTradeField)  &emsp;&emsp;&emsp;&emsp; 成交回报数据结构

常量
====
* [SOURCE](#SOURCE) &emsp;&emsp;&emsp;&emsp; 接口常量
* [DIRECTION](#DIRECTION) &emsp;&emsp;&emsp;&emsp; 买卖方向常量
* [OFFSET](#OFFSET) &emsp;&emsp;&emsp;&emsp; 开平方向常量
* [PRICE_TYPE](#PRICE_TYPE) &emsp;&emsp;&emsp;&emsp; 报单价格类型常量
* [ORDER_STATUS](#ORDER_STATUS) &emsp;&emsp;&emsp;&emsp; 订单状态常量
* [POS_DIRECTION](#POS_DIRECTION) &emsp;&emsp;&emsp;&emsp; 持仓方向常量
* [VOLUME_TYPE](#VOLUME_TYPE) &emsp;&emsp;&emsp;&emsp; 订单交易量限制常量
* [TIME_CONDITION](#TIME_CONDITION) &emsp;&emsp;&emsp;&emsp; 订单交易时间限制常量

函数定义
=======

### 回调函数

<h3 id="initialize">initialize(context)</h3>

功能: 策略初始化的时候被回调，可在该函数中连接行情和交易，订阅行情等.   

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|


<h3 id="on_pos"> on_pos(context, pos_handler, request_id, souce, rcv_time) </h3>

功能: 当策略连接到TD时回调一次，回报 request_id 为-1，如果策略首次运行，pos_handler 为None，如果策略之前运行且设置过持仓，pos_handler 为策略持仓。该函数在该策略查询了持仓以后会回调，request_id 为查询持仓时的请求序号，pos_handler 为账户持仓    

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|pos_handler|PosHandler|持仓数据|
|request_id|int|请求编号|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|

<h3 id="on_tick">on_tick(context, md, souce, rcv_time)</h3>

功能: 收到tick行情数据时回调，需要订阅行情  

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|md|LFMarketDataField|行情数据|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|

<h3 id="on_bar">on_bar(context, bars, souce, rcv_time)</h3>
功能: 收到分钟线数据的时回调，需要订阅分钟线行情      

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|bars|dictionary((ticker(str) -> LFBarMarketDataField))|分钟线行情|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|


<h3 id="on_rtn_order"> on_rtn_order(context, rtn_order, order_id, souce, rcv_time) </h3>

功能: 收到本策略下单的下单回报时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|rtn_order|[LFRtnOrderField](#LFRtnOrderField)|下单返回数据结构|
|order_id|int|下单编号|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|

<h3 id="on_rtn_trade"> on_rtn_trade(context, rtn_trade, order_id, souce, rcv_time) </h3>

功能: 收到本策略下单的成交回报时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|rtn_trade|[LFRtnTradeField](#LFRtnTradeField)|成交返回数据结构|
|order_id|int|下单编号|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|

<h3 id="on_error"> on_error(context, error_id, error_msg, request_id, souce, rcv_time) </h3>

功能:当收到错误回报时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|context|context|系统定义用于调用功能函数和存放任意数据的综合体|
|error_id|int|错误ID|
|error_msg|string|错误信息|
|request_id|int|请求编号|
|souce|[SOURCE](#SOURCE)|行情交易接口常量|
|rcv_time|long|返回纳秒时间|

## 功能函数

<h3 id="context.add_md">context.add_md(source)</h3>

功能: 连接行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|

<h3 id="context.add_td">context.add_td(source)</h3>

功能: 连接交易

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|

<h3 id="context.req_pos">context.req_pos(source)</h3>

功能: 查询账户持仓，如果查询失败返回 -1，查询成功则返回 request_id ，当收到交易所返回持仓时通过回调 on_pos 返回持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|返回|返回值类型|返回值说明|
|request_id|int|请求编号|

<h3 id="context.new_pos">context.new_pos(source)</h3>

功能:创建一个空的持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|返回值|类型|参数说明|
|pos_handler|PosHandler|持仓数据结构(空持仓)|

<h3 id="context.set_pos">context.set_pos(pos_handler, source)</h3>

功能: 设置策略持仓。

|参数|类型|参数说明|
|:--:|:--:|:--:|
|pos_handler|PosHandler|持仓数据结构|
|source|[SOURCE](#SOURCE)|行情交易接口常量|

<h3 id="context.get_pos">context.get_pos(source)</h3>

功能:获得策略持仓 

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|返回|返回值类型|返回值说明|
|pos_handler|PosHandler|持仓数据结构|

<h3 id="context.print_pos">context.print_pos(pos_handler)</h3>

功能: 打印持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|pos_handler|PosHandler|持仓数据|

<h3 id="context.subscribe">context.subscribe(tick_list, source)</h3>

功能: 订阅行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|tickers|list|订阅合约列表|
|source|[SOURCE](#SOURCE)|行情交易接口常量|

<h3 id="context.register_bar">context.register_bar(source, min_inerval, start_time, ent_time)</h3>

功能: 订阅分钟线行情，间隔时间可为自定义整数

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|min_inerval|int|bar间隔分钟数|
|start_time|string|bar数据开始时间|
|ent_time|string|bar数据结束时间|

<h3 id="context.stop">context.stop()</h3>

功能: 停止策略

<h3 id="context.get_nano">context.get_nano()</h3>

功能: 获得当前纳秒时间

|返回|返回值类型|返回值说明|
|:--:|:--:|:--:|
|nano|long|系统当前纳秒时间|

<h3 id="context.get_name">context.get_name()</h3>

功能: 获取策略名

|返回|返回值类型|返回值说明|
|:--:|:--:|:--:|
|stre_name|string|策略名称|

<h3 id="context.parse_time">context.parse_time(time_str)</h3>

功能: 将时间字符串转换为纳秒时间 

|参数|类型|参数说明|
|:--:|:--:|:--:|
|time_str|string|时间字符串("%Y-%m-%d %H:%M:%S")|
|返回|返回值类型|返回值说明|
|nano|long|纳秒时间|

<h3 id="context.parse_nano">context.parse_nano(nano)</h3>

功能: 将纳秒时间转换为字符串时间

|参数|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|纳秒时间|
|返回|返回值类型|返回值说明|
|time_str|string|时间字符串("%Y-%m-%d %H:%M:%S")|

<h3 id="context.insert_market_order">context.insert_market_order(source, ticker, exchange_id, volume, direction, offset)</h3>

功能: 下市价单，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|ticker|string|合约代码|
|exchange_id|string|交易所代码|
|volume|int|数量|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|offset|[OFFSET](#OFFSET)|开平方向|
|返回|返回值类型|返回值说明|
|order_id|int|下单编号|

<h3 id="context.insert_limit_order">context.insert_limit_order(source, ticker, exchange_id, price, volume, direction, offset)</h3>

功能: 下限价单，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|ticker|string|合约代码|
|exchange_id|string|交易所代码|
|price|float|价格|
|volume|int|数量|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|offset|[OFFSET](#OFFSET)|开平方向|
|返回|返回值类型|返回值说明|
|order_id|int|下单编号|

<h3 id="context.insert_fok_order">context.insert_fok_order(source, ticker, exchange_id, price, volume, direction, offset)</h3>

功能: 下 FOK 类型订单（全部成交或则全部撤销），如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|ticker|string|合约代码|
|exchange_id|string|交易所代码|
|price|float|价格|
|volume|int|数量|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|offset|[OFFSET](#OFFSET)|开平方向|
|返回|返回值类型|返回值说明|
|order_id|int|下单编号|

<h3 id="context.insert_fak_order">context.insert_fak_order(source, ticker, exchange_id, price, volume, direction, offset)</h3>

功能: 下 FAK 类型订单（成交且位成交部分撤销），如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|ticker|string|合约代码|
|exchange_id|string|交易所代码|
|price|float|价格|
|volume|int|数量|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|offset|[OFFSET](#OFFSET)|开平方向|
|返回|返回值类型|返回值说明|
|order_id|int|下单编号|

<h3 id="context.cancel_order">context.cancel_order(source, order_id)</h3>

功能: 使用下单编号撤销订单，如果失败则返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|[SOURCE](#SOURCE)|行情交易接口常量|
|order_id|int|下单编号|
|返回|返回值类型|返回值说明|
|request_id|int|请求编号|

<h3 id="context.insert_func_at">context.context.insert_func_at(nano
, function)</h3>

功能: 在指定纳秒时间添加回调函数

|参数|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|回调纳秒时间|
|function|func|回调函数|

<h3 id="context.insert_func_at_c">context.insert_func_at_c(nano
, function)</h3>

功能: 在指定纳秒时间添加以 context 为参数的回调函数

|参数|类型|参数说明|
|:--:|:--:|:--:|
| nano |long|回调纳秒时间|
|function|func|回调函数（以 context 为默认参数）|


<h3 id="context.insert_func_after">context.insert_func_after(seconds
, function)</h3>

功能: 在指定秒数后添加回调函数

|参数|类型|参数说明|
|:--:|:--:|:--:|
|seconds|int|需要在多少秒之后回调|
|function|func|回调函数|

<h3 id="context.insert_func_after_c">context.insert_func_after_c(seconds
, function)</h3>

功能: 在指定秒数后添加以 context 为参数的回调函数

|参数|类型|参数说明|
|:--:|:--:|:--:|
|nano[long]|long|回调纳秒时间|
|function|func|回调函数（以 context 为默认参数）|

<h3 id="context.log_debug">context.log_debug(msg)</h3>

功能: 输出 debug 日志

|参数|类型|参数说明|
|:--:|:--:|:--:|
|msg|string|日志字符串|

<h3 id="context.log_info">context.log_info(msg)</h3>

功能: 输出 info 日志

|参数|类型|参数说明|
|:--:|:--:|:--:|
|msg|string|日志字符串|

<h3 id="context.log_error">context.log_error(msg)</h3>

功能: 输出 error 日志

|参数|类型|参数说明|
|:--:|:--:|:--:|
|msg|string|日志字符串|

<h3 id="context.log_fatal">context.log_fatal(msg)</h3>

功能: 输出 fatal 日志

|参数|类型|参数说明|
|:--:|:--:|:--:|
|msg|string|日志字符串|

## 持仓成员函数

<h3 id="PosHandler.add_pos">PosHandler.add_pos(ticker, direction, tot_pos, yd_pos)</h3>

功能: 向持仓数据项中添加持仓增量

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|需要增加仓位的合约号|
|direction|[POS_DIRECTION](#POS_DIRECTION)|仓位方向|
|tot_pos|int|总仓位|
|yd_pos|int|昨仓位|


<h3 id="PosHandler.set_pos">PosHandler.set_pos(ticker, direction, tot_pos, yd_pos)</h3>

功能: 向持仓数据项中添加持仓（会替换原有项）

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|需要增加仓位的合约号|
|direction|[POS_DIRECTION](#POS_DIRECTION)|仓位方向|
|tot_pos|int|总仓位|
|yd_pos|int|昨仓位|

<h3 id="PosHandler.dump">PosHandler.dump()</h3>

功能: 持仓转换为 json 格式字符串

|返回|返回值类型|返回值说明|
|:--:|:--:|:--:|
|pos_str|string|json 标准格式的持仓字符串|

<h3 id="PosHandler.load">PosHandler.load(pos_str)</h3>
功能: 将 json 格式的持仓数据设置为持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|string|string|json 标准格式的持仓字符串|


<h3 id="PosHandler.get_tickers">PosHandler.get_tickers()</h3>

功能: 获得持仓数据的合约列表

|返回|返回值类型|返回值说明|
|:--:|:--:|:--:|
|tickers|list|有持仓的合约list|

<h3 id="PosHandler.get_net_tot">PosHandler.get_net_tot(ticker)</h3>
功能: 查询指定股票总持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定股票总持仓|

<h3 id="PosHandler.get_net_yd">PosHandler.get_net_yd(ticker)</h3>
功能: 查询指定股票昨仓

|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定股票昨仓|

<h3 id="PosHandler.get_long_tot">PosHandler.get_long_tot(ticker)</h3>
功能: 查询指定期货某合约做多总持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定合约总多仓|

<h3 id="PosHandler.get_long_yd">PosHandler.get_long_yd(ticker)</h3>
功能: 查询指定期货某合约做多昨仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定合约昨多仓|

<h3 id="PosHandler.get_short_tot">PosHandler.get_short_tot(ticker)</h3>
功能: 查询指定期货某合约做空总持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定合约总空仓|

<h3 id="PosHandler.get_short_yd">PosHandler.get_short_yd(ticker)</h3>
功能: 查询指定期货某合约做空昨仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ticker|string|要获取仓位的合约|
|返回|返回值类型|返回值说明|
|pos|int|指定合约昨空仓|
	
数据结构定义
==========

<h3 id="LFMarketDataField">LFMarketDataField</h3>

内容: 行情数据结构

|成员变量|类型|变量说明|
|:--:|:--:|:--:|
|TradingDay|string|交易日|
|InstringumentID|string|合约代码|
|ExchangeID|string|交易所代码|
|ExchangeInstID|string|合约在交易所的代码|
|LastPrice|float|最新价|
|PreSettlementPrice|float|上次结算价|
|PreClosePrice|float|昨收盘|
|PreOpenInterest|float|昨持仓量|
|OpenPrice|float|今开盘|
|HighestPrice|float|最高价|
|LowestPrice|float|最低价|
|Volume|int|数量|
|Turnover|float|成交金额|
|OpenInterest|float|持仓量|
|ClosePrice|float|今收盘|
|SettlementPrice|float|本次结算价|
|UpperLimitPrice|float|涨停板价|
|LowerLimitPrice|float|跌停板价|
|PreDelta|float|昨虚实度|
|CurrDelta|float|今虚实度|
|UpdateTime|string|最后修改时间|
|UpdateMillisec|int|最后修改毫秒|
|BidPrice1|float|申买价一|
|BidVolume1|int|申买量一|
|AskPrice1|float|申卖价一|
|AskVolume1|int|申卖量一|
|BidPrice2|float|申买价二|
|BidVolume2|int|申买量二|
|AskPrice2|float|申卖价二|
|AskVolume2|int|申卖量二|
|BidPrice3|float|申买价三|
|BidVolume3|int|申买量三|
|AskPrice3|float|申卖价三|
|AskVolume3|int|申卖量三|
|BidPrice4|float|申买价四|
|BidVolume4|int|申买量四|
|AskPrice4|float|申卖价四|
|AskVolume4|int|申卖量四|
|BidPrice5|float|申买价五|
|BidVolume5|int|申买量五|
|AskPrice5|float|申卖价五|
|AskVolume5|int|申卖量五|

<h3 id="LFBarMarketDataField">LFBarMarketDataField</h3>

内容: 分钟线数据结构

|成员变量|类型|变量说明|
|:--:|:--:|:--:|
|TradingDay|string|交易日|
|InstringumentID|string|合约代码|
|UpperLimitPrice|float|涨停板价|
|LowerLimitPrice|float|跌停板价|
|StartUpdateTime|string|首tick修改时间|
|StartUpdateMillisec|int|首tick最后修改毫秒|
|EndUpdateTime|string|尾tick最后修改时间|
|EndUpdateMillisec|int|尾tick最后修改毫秒|
|Open|float|开|
|Close|float|收|
|Low|float|最高价|
|High|float|最低价|
|Volume|float|区间交易量|
|StartVolume|float|初始总交易量|

<h3 id="LFRtnOrderField">LFRtnOrderField</h3>

内容: 下单回报数据结构

|成员变量|类型|变量说明|
|:--:|:--:|:--:|
|BrokerID|string|经纪公司代码|
|ParticipantID|string|会员代码|
|InvestorID|string|投资者代码|
|BusinessUnit|string|业务单元|
|InstringumentID|string|合约代码|
|OrderRef|string|报单引用|
|ExchangeID|string|交易所代码|
|LimitPrice|float|价格|
|VolumeTraded|int|今成交数量|
|VolumeTotal|int|剩余数量|
|VolumeTotalOriginal|int|数量|
|TimeCondition|[TIME_CONDITION](#TIME_CONDITION)|有效期类型|
|VolumeCondition|[VOLUME_TYPE](#VOLUME_TYPE)|成交量类型|
|OrderPriceType|[PRICE_TYPE](#PRICE_TYPE)|报单价格条件|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|OffsetFlag|[OFFSET](#OFFSET)|开平标志|
|OrderStatus|[ORDER_STATUS](#ORDER_STATUS)|报单状态|

<h3 id="LFRtnTradeField">LFRtnTradeField</h3>

内容: 成交回报数据结构

|成员变量|类型|变量说明|
|:--:|:--:|:--:|
|BrokerID|string|经纪公司代码|
|InvestorID|string|投资者代码|
|BusinessUnit|string|业务单元|
|InstringumentID|string|合约代码|
|OrderRef|string|报单引用|
|ExchangeID|string|交易所代码|
|Price|float|价格|
|Volume|int|数量|
|TradingDay|string|交易日|
|TradeTime|string|成交时间|
|direction|[DIRECTION](#DIRECTION)|买卖方向|
|OffsetFlag|[OFFSET](#OFFSET)|开平标志|

常量定义
=======

<h3 id="SOURCE">SOURCE</h3>

内容: 接口常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|CTP|1||
|XTP|15||

<h3 id="DIRECTION"> DIRECTION </h3>

功能: 买卖方向常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|Buy|‘1'|买|
|Sell|‘2'|卖|

<h3 id="OFFSET"> OFFSET </h3>

功能: 开平方向常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|Open|‘1'|开仓|
|Close|‘2'|平仓|
|CloseToday|‘3'|平今|
|CloseYesterday|‘4'|平昨|

<h3 id="PRICE_TYPE"> PRICE_TYPE </h3>

功能: 报单价格类型常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|AnyPrice|‘1'|任意价|
|LimitPrice|‘2'|限价|
|BestPrice|‘3'|最优价|

<h3 id="ORDER_STATUS"> ORDER_STATUS </h3>

功能: 订单状态常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|AllTraded|‘0'|全部成交（最终状态）|
|PartTradedQueueing|‘1'|部分成交还在队列中|
|PartTradedNotQueueing|‘2'|部分成交不在队列中（部成部撤，最终状态）|
|NoTradeQueueing|‘3'|未成交还在队列中|
|NoTradeNotQueueing|‘4'|未成交不在队列中（被拒绝，最终状态）|
|Canceled|‘5'|撤单|
|Unknown|‘a'|未知|

<h3 id="POS_DIRECTION"> POS_DIRECTION </h3>

功能: 持仓方向常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|Net|‘1'|净|
|Long|‘2'|多头|
|Short|‘3'|空头|

<h3 id="VOLUME_TYPE"> VOLUME_TYPE </h3>

功能: 订单交易量限制常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|AV|‘1'|任何数量|
|MV|‘2'|最小数量|
|CV|‘3'|全部数量|

<h3 id="TIME_CONDITION"> TIME_CONDITION </h3>

功能: 订单交易时间限制常量

|标记|值|常量含义|
|:--:|:--:|:--:|
|IOC|‘1'|立即完成，否则撤销|
|GFS|‘2'|本节有效|
|GFD|‘3'|当日有效|
|GTD|‘4'|指定日期前有效|
|GTC|‘5'|撤销前有效|
|GFA|‘6'|集合竞价有效|