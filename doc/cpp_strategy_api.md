# 函数列表
* 初始化函数
  * [init](#init) &emsp;&emsp;&emsp;&emsp; 初始化函数
  
* 回调函数 
  * [on_market_data](#on_market_data)  &emsp;&emsp;&emsp;&emsp; tick 行情回调函数
  * [on_market_bar](#on_market_bar)&emsp;&emsp;&emsp;&emsp; 分钟线行情回调函数
  * [on_market_data_level2](#on_market_data_level2) &emsp;&emsp;&emsp;&emsp; 二档行情回调函数
  * [on_l2_index](#on_l2_index) &emsp;&emsp;&emsp;&emsp; 指数信息回调函数
  * [on_l2_order](#on_l2_order) &emsp;&emsp;&emsp;&emsp; 逐笔委托信息回调函数
  * [on_l2_trade](#on_l2_trade) &emsp;&emsp;&emsp;&emsp; 逐笔成交信息回调函数
  * [on_rtn_order](#on_rtn_order) &emsp;&emsp;&emsp;&emsp; 下单回报回调函数 
  * [on_rtn_trade](#on_rtn_trade) &emsp;&emsp;&emsp;&emsp; 成交回报回调函数 
  * [on_rsp_order](#on_rsp_order) &emsp;&emsp;&emsp;&emsp; 错误消息回调函数 
  * [on_rsp_position](#on_rsp_position) &emsp;&emsp;&emsp;&emsp; 持仓返回回调函数
  * [on_switch_day](#on_switch_day) &emsp;&emsp;&emsp;&emsp; 换日回调函数
  * [on_td_login](#on_td_login) &emsp;&emsp;&emsp;&emsp; 连接交易接口状态返回回调函数

* WCDataWrapper 成员函数
  * [add_market_data](#add_market_data) &emsp;&emsp;&emsp;&emsp; 连接行情服务
  * [add_market_data_l2](#data.add_market_data_l2)  &emsp;&emsp;&emsp;&emsp; 连接二档行情服务
  * [add_l2_index](#add_l2_index)  &emsp;&emsp;&emsp;&emsp; 连接指数信息服务
  * [add_l2_order](#add_l2_order)  &emsp;&emsp;&emsp;&emsp; 连接逐笔委托信息服务
  * [add_l2_trade](#add_l2_trade)  &emsp;&emsp;&emsp;&emsp; 连接逐笔成交信息服务
  * [add_register_td](#add_register_td)  &emsp;&emsp;&emsp;&emsp; 连接交易服务
  * [register_bar_md](#register_bar_md)  &emsp;&emsp;&emsp;&emsp; 订阅分钟线行情
  * [run](#run)  &emsp;&emsp;&emsp;&emsp; 启动策略(行情和交易)
  * [stop](#stop)  &emsp;&emsp;&emsp;&emsp; 停止策略
  * [get_td_status](#get_td_status) &emsp;&emsp;&emsp;&emsp; 获取策略连接的交易接口状态
  * [set_pos](#set_pos) &emsp;&emsp;&emsp;&emsp; 设置策略持仓
  * [get_ticker_pnl](#get_ticker_pnl) &emsp;&emsp;&emsp;&emsp; 获取某合约或股票的持仓盈亏
  * [get_effective_orders](#get_effective_orders) &emsp;&emsp;&emsp;&emsp; 获取未全部成交的下单请求序号列表

* WCStrategyUtil 成员函数
  * [subscribe_market_data](subscribe_market_data) &emsp;&emsp;&emsp;&emsp; 订阅行情
  * [subscribe_market_data_l2](subscribe_market_data_l2) &emsp;&emsp;&emsp;&emsp; 订阅二档行情
  * [subscribe_l2_index](subscribe_l2_index) &emsp;&emsp;&emsp;&emsp; 订阅指数信息
  * [subscribe_l2_order](subscribe_l2_order) &emsp;&emsp;&emsp;&emsp; 订阅逐笔委托信息
  * [subscribe_l2_trade](subscribe_l2_trade) &emsp;&emsp;&emsp;&emsp; 订阅逐笔成交信息
  * [insert_market_order](#insert_market_order) &emsp;&emsp;&emsp;&emsp; 下市价单
  * [insert_limit_order](#insert_limit_order) &emsp;&emsp;&emsp;&emsp; 下限价单
  * [insert_fok_order](#insert_fok_order) &emsp;&emsp;&emsp;&emsp; 下FOK类型订单（全部成交或则全部撤销），如果失败返回-1
  * [insert_fak_order](#insert_fak_order) &emsp;&emsp;&emsp;&emsp; 下FAK类型订单（成交且为成交部分撤销），如果失败返回-1
  * [req_position](#req_position) &emsp;&emsp;&emsp;&emsp; 查询账户持仓
  * [cancel_order](#cancel_order) &emsp;&emsp;&emsp;&emsp; 撤消指定合约
  * [insert_callback](#insert_callback) &emsp;&emsp;&emsp;&emsp; 添加回调函数（python）
  * [set_pos_back](#set_pos_back) &emsp;&emsp;&emsp;&emsp; 设置策略持仓
  * [get_nano](#get_nano) &emsp;&emsp;&emsp;&emsp; 获取当前纳秒时间
  * [get_time](#get_time) &emsp;&emsp;&emsp;&emsp; 获取当前时间
  * [parse_time](#parse_time) &emsp;&emsp;&emsp;&emsp; 将字符串时间转换为纳秒时间
  * [parse_nano](#parse_nano) &emsp;&emsp;&emsp;&emsp; 将纳秒时间转换为字符串时间


## 初始化函数

<h3 id="init">init()</h3>

功能描述：策略初始化

## 回调函数

<h3 id="on_market_data">on_market_data(const LFMarketDataField* data, short source, long rcv_time)</h3>

功能： 收到tick行情数据时回调，需要前面有订阅行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFMarketDataField *|tick 行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_market_bar">on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time)</h3>

功能：收到分钟线信息的时回调，需要前面有订阅分钟线行情 

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|BarMdMap&|bar 行情数据|
|min_interval|int|bar 行情分钟间隔|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_market_data_level2">on_market_data_level2(const LFL2MarketDataField* data, short source, long rcv_time)</h3>

功能：收到二档行情时回调，需要行情接口支持二档行情且订阅了二档行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2MarketDataField*|二档行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_l2_index">on_l2_index(const LFL2IndexField* data, short source, long rcv_time)</h3>

功能： 收到指数信息时回调，需要行情接口支持指数信息且订阅了指数行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2IndexField|指数行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|


<h3 id="on_l2_order">on_l2_order(const LFL2OrderField* data, short source, long rcv_time)</h3>

功能：收到逐笔委托信息时回调，需要行情接口支持逐笔委托信息且订阅了逐笔委托数据

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2OrderField*|逐笔委托数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_l2_trade">on_l2_trade(const LFL2TradeField* data, short source, long rcv_time)</h3>

功能：收到逐笔成交信息时回调，需要行情接口支持逐笔成交信息且订阅了逐笔成交数据

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2TradeField*|逐笔成交数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_rtn_order">on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time)</h3>

功能：收到本策略下单的下单回报时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFRtnOrderField*|下单回报数据|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_rtn_trade">on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time)</h3>

功能：收到本策略下单的成交回报时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFRtnTradeField*|成交回报数据|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_rsp_order">on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr)</h3>

收到本策略下单（错误）信息返回时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|data|LFInputOrderField* |报单数据结构|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|
|errorId|short|错误 ID|
|errorMsg| char* |错误信息|

<h3 id="on_rsp_position">on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time)</h3>

功能：当策略连接到TD时回调一次，回报request_id为-1，如果策略首次运行，pos_handler为空值，如果策略之前运行且设置过持仓，pos_handler为策略持仓。同时该函数在该策略查询了持仓以后会回调，request_id为查询持仓时的请求序号，pos_handler为账户持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|posMap|PosHandlerPtr|持仓信息|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_switch_day">on_switch_day(long rcv_time)</h3>

功能：在设置的换天时间时回调

|参数|类型|参数说明|
|:--:|:--:|:--:|
|rcv_time|long|函数被调用的纳秒时间|

<h3 id="on_td_login">on_td_login(bool ready, const json& js, short source)</h3>

功能：策略连接交易接口返回

|参数|类型|参数说明|
|:--:|:--:|:--:|
|ready|bool|是否连接成功|
|js|json&|持仓json格式文件|
|source|short|交易接口标记|


# WCDataWrapper成员函数

C++策略基类有一个类型为 WCDataWrapper 的 data 成员对象，用户可通过该对象完成连接到行情等操作

<h3 id="add_market_data">add_market_data(short source)</h3>

功能：给策略添加行情服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_market_data_l2">add_market_data_l2(short source)</h3>

功能：给策略添加二档行情服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_index">add_l2_index(short source)</h3>

功能：给策略添加指数行情服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_order">add_l2_order(short source)</h3>

功能：给策略添加下单行情服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_trade">add_l2_trade(short source)</h3>

功能：给策略添加成交行情服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_register_td">add_register_td(short source)</h3>

功能：给策略添加交易服务

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|



<h3 id="register_bar_md">register_bar_md(short source, int min_interval, string start_time, string end_time)</h3>

功能：添加分钟线行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|min_interval|int|bar间隔分钟数|
|start_time|string|bar数据开始时间|
|end_time|string|bar数据结束时间|

<h3 id="run">run()</h3>

功能：启动策略数据过程

<h3 id="stop">stop()</h3>

功能：停止策略

<h3 id="get_td_status">get_td_status(short source)</h3>

功能：获取策略连接的交易接口状态

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||byte|td状态|

<h3 id="set_pos">set_pos(PosHandlerPtr pos, short source)</h3>

功能：获取策略连接的交易接口状态

|参数|类型|参数说明|
|:--:|:--:|:--:|
|pos|PosHandlerPtr|持仓数据|
|source|short|行情交易接口常量|



<h3 id="get_ticker_pnl">get_ticker_pnl(short source, string ticker, bool include_fee=true)</h3>

功能：获取某合约或股票的持仓盈亏

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|ticker|string|合约或股票编号|
|include_fee|bool|是否包含交易费用|
|返回值|类型|返回值说明|
||double|该合约或股票持仓盈亏|

<h3 id="get_effective_orders">get_effective_orders()</h3>

功能：获取有效（下单未全部成交）合约

|返回值|类型|返回值说明|
|:--:|:--:|:--:|
||vector<int>|下单请求编号列表|


# WCStrategyUtil成员函数

C++策略基类有一个类型为 WCStrategyUtil 的 util 成员对象，用户可通过该对象完成订阅、下单等操作

<h3 id="subscribe_market_data">subscribe_market_data(boost::python::list tickers, short source)</h3>

功能：订阅行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
| tickers |boost::python::list|合约列表|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||bool|订阅函数是否成功|

<h3 id="subscribe_market_data_l2">subscribe_market_data_l2(boost::python::list tickers, short source)</h3>

功能：订阅二档行情

|参数|类型|参数说明|
|:--:|:--:|:--:|
| tickers |boost::python::list|合约列表|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||bool|订阅函数是否成功|

<h3 id="subscribe_l2_index">subscribe_l2_index(boost::python::list tickers, short source)</h3>

功能：订阅指数信息

|参数|类型|参数说明|
|:--:|:--:|:--:|
| tickers |boost::python::list|合约列表|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||bool|订阅函数是否成功|

<h3 id="subscribe_l2_order">subscribe_l2_order(boost::python::list tickers, short source)</h3>

功能：订阅逐笔成交信息

|参数|类型|参数说明|
|:--:|:--:|:--:|
| tickers |boost::python::list|合约列表|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||bool|订阅函数是否成功|

<h3 id="subscribe_l2_trade">subscribe_l2_trade(boost::python::list tickers, short source)</h3>

功能：订阅逐笔成交信息

|参数|类型|参数说明|
|:--:|:--:|:--:|
| tickers |boost::python::list|合约列表|
|source|short|行情交易接口常量|
|返回值|类型|返回值说明|
||bool|订阅函数是否成功|

<h3 id="insert_market_order">insert_market_order(short source, string instrument_id, string exchange_id, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下市价单，需要交易所和交易接口支持，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|返回值|类型|返回值说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_limit_order">insert_limit_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下限价单，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|返回值|类型|返回值说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_fok_order">insert_fok_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下 FOK 类型订单（全部成交或则全部撤销），需要交易所和交易接口支持，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|返回值|类型|返回值说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_fak_order">insert_fak_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下 FAK 类型订单（成交且为成交部分撤销），需要交易所和交易接口支持，如果失败返回-1

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|返回值|类型|返回值说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="req_position">req_position(short source)</h3>

功能：查询账户持仓，持仓数据通过on_rsp_position回调返回

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|
|返回值|类型|返回值说明|
||int|查询请求序号，若失败返回-1|

<h3 id="cancel_order">cancel_order(short source, int order_id)</h3>

功能：撤消指定合约

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|order_id|int|要撤销的订单的下单请求编号|
|返回值|类型|返回值说明|
|request_id|int|请求编号|

<h3 id="insert_callback">insert_callback(long nano, BLCallback& callback)</h3>

功能：添加回调函数

|参数|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|回调纳秒时间|
|callback| BLCallback&|回调函数|
|返回值|类型|返回值说明|
||bool|回调函数是否添加成功|

<h3 id="insert_callback_py">insert_callback_py(long nano, boost::python::object func)</h3>

功能：添加回调函数（python）

|参数|类型|回调|
|nano|nano|回调纳秒时间|
|func|boost::python::object|回调函数|
|返回值|类型|返回值说明|
||bool|回调函数是否添加成功|

<h3 id="set_pos_back">set_pos_back(short source, const char* pos_str)</h3>

功能：设置策略持仓

|参数|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|
|pos_str|char*|持仓信息字符串|

<h3 id="get_nano">get_nano()</h3>

功能：获取当前纳秒时间

|返回值|类型|返回值说明|
|:--:|:--:|:--:|
||long|当前纳秒时间|

<h3 id="get_time">get_time()</h3>

功能：获取当前时间

|返回值|类型|返回值说明|
|:--:|:--:|:--:|
||string|时间字符串|

<h3 id="parse_time">parse_time(string time_str)</h3>

功能：将字符串时间转换为纳秒时间

|参数|类型|参数说明|
|:--:|:--:|:--:|
|time_str|string|字符串时间|
|返回值|类型|返回值说明|
||long|纳秒时间|

<h3 id="parse_nano">parse_nano(long nano)</h3>

功能：将纳秒时间转换为字符串时间

|参数|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|纳秒时间|
|返回值|类型|返回值说明|
||string|字符串时间|