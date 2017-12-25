# 函数列表
* 回调函数 
  * [on_market_data](#on_market_data)回调返回tick行情数据
  * [on_market_data_level2](*on_market_data_level2)回调返回L2行情数据
  * [on_l2_index](*on_l2_index)回调返回指数数据
  * [on_l2_order](*on_l2_order)回调返回市场下单数据
  * [on_l2_trade](*on_l2_trade)回调返回市场成交数据
  * [on_market_bar](*on_market_bar)回调返回分钟线行情
  * [on_rtn_order](*on_rtn_order)下单回报
  * [on_rtn_trade](*on_rtn_trade)成交回报
  * [on_rsp_order](*on_rsp_order)下单错误回报
  * [on_rsp_position](*on_rsp_position)持仓回报
  * [on_switch_day](*on_switch_day)换天回调
  * [on_time](*on_time)
  * [on_td_login](*on_td_login)策略注册交易接口返回

* 功能函数
  * [init](*init)
  * [debug](*debug) 
  * [get_name](*get_name)

* data成员函数
  * [add_market_data](*add_market_data)给策略添加行情服务
  * [data.add_market_data_l2](*data.add_market_data_l2) 给策略添加二档行情服务
  * [add_l2_index](*add_l2_index)给策略添加指数行情服务
  * [data.add_l2_order](*data.add_l2_order)给策略添加下单行情服务
  * [data.add_l2_trade](*data.add_l2_trade)给策略添加成交行情服务
  * [add_register_td](*add_register_td)给策略添加交易服务
  * [register_bar_md](*register_bar_md)添加分钟线行情
  * [run](*run)启动策略数据过程
  * [stop](*stop)停止策略
  * [get_td_status](*get_td_status) 获取策略连接的交易接口状态
  * [set_pos](*set_pos) 设置策略持仓
  * [get_ticker_pnl](*get_ticker_pnl)获取某合约或股票的持仓盈亏
  * [get_effective_orders](*get_effective_orders)获取有效（下单未全部成交）合约

* util成员函数
  * [insert_market_order](*insert_market_order)下市价单
  * [insert_limit_order](*insert_limit_order)下限价单
  * [insert_fok_order](*insert_fok_order)下FOK类型订单（全部成交或则全部撤销），如果失败返回-1
  * [insert_fak_order](*insert_fak_order)下FAK类型订单（成交且为成交部分撤销），如果失败返回-1
  * [req_position](*req_position)查询账户持仓
  * [cancel_order](*cancel_order)撤消指定合约
  * [process_callback](*process_callback)
  * [insert_callback](*insert_callback)添加回调函数（python）
  * [set_md_nano](*set_md_nano)
  * [set_pos_back](*set_pos_back)设置策略持仓
  * [get_nano](*get_nano)获取当前纳秒时间
  * [get_time](*get_time)获取当前时间
  * [parse_time](*parse_time)将字符串时间转换为纳秒时间
  * [parse_nano](*parse_nano)将纳秒时间转换为字符串时间


## 回调函数

<h3 id="on_market_data">on_market_data(const LFMarketDataField* data, short source, long rcv_time)</h3>

功能： 回调返回tick行情数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFMarketDataField *|tick行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_market_bar">on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time)</h3>

功能：回调返回L2行情数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2MarketDataField*|L2行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_market_data_level2">on_market_data_level2(const LFL2MarketDataField* data, short source, long rcv_time)</h3>

功能：回调返回L2行情数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2MarketDataField*|L2行情数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_l2_index">on_l2_index(const LFL2IndexField* data, short source, long rcv_time)</h3>

功能： 回调返回指数数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2IndexField|指数行情数据结构|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|


<h3 id="on_l2_order">on_l2_order(const LFL2OrderField* data, short source, long rcv_time)</h3>

功能：回调返回市场下单数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2OrderField*|委托数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_l2_trade">on_l2_trade(const LFL2TradeField* data, short source, long rcv_time)</h3>

功能：回调返回市场成交数据

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFL2TradeField*|成交数据|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_market_bar">on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time)</h3>

功能：回调返回分钟线行情

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|BarMdMap&|bar行情数据|
|min_interval|int|bar行情分钟间隔|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|


<h3 id="on_rtn_order">on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time)</h3>

功能：收到策略下单的下单回报时被触发，非本策略下单的下单回报不会触发该函数

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFRtnOrderField*|下单回报数据|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|


<h3 id="on_rtn_trade">on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time)</h3>

功能：收到策略下单的成交回报时被触发，非本策略下单的成交回报不会触发该函数

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFRtnTradeField*|成交回报数据|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|


<h3 id="on_rsp_order">on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr)</h3>
收到下单（错误）信息返回时被触发，非本策略下单的成交回报不会触发该函数

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|data|LFInputOrderField*|报单数据结构|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|
|errorId|short|错误ID|
|errorMsg| char* |错误信息|

<h3 id="on_rsp_position">on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time)</h3>

功能：当策略注册到TD时被触发一次，回报request_id为-1，如果策略首次运行，pos_handler为空值，如果策略之前运行且设置过持仓，pos_handler为策略持仓。同时该函数在该策略查询了持仓以后会被触发，request_id为查询持仓时的请求序号，pos_handler为账户持仓

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|posMap|PosHandlerPtr|持仓信息|
|request_id|int|请求编号|
|source|short|行情数据源|
|rcv_time|long|收到行情纳秒时间|

<h3 id="on_switch_day">on_switch_day(long rcv_time)</h3>

功能：在设置的换天时间时被触发

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|rcv_time|long|函数被调用的纳秒时间|

<h3 id="on_time">on_time(long cur_time)</h3>

功能描述

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|cur_time|long|函数被调用的纳秒时间|

<h3 id="on_td_login">on_td_login(bool ready, const json& js, short source)</h3>

功能：策略注册交易接口返回

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|ready|bool|是否登录成功|
|js|json&|持仓json格式文件|
|source|short|交易接口标记|

## 功能函数

<h3 id="debug">debug(const char* msg)</h3>

功能描述：输出debug日志

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|msg|char*|debug日志内容|

<h3 id="get_name">get_name()</h3>

功能： 获得策略名

|输出参数名|类型|参数说明|
|:--:|:--:|:--:|
|name|string|策略名|

<h3 id="init">init()</h3>

功能描述：策略初始化



# data成员函数

<h3 id="add_market_data">add_market_data(short source)</h3>

功能：给策略添加行情服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_market_data_l2">add_market_data_l2(short source)</h3>

功能：给策略添加二档行情服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_index">add_l2_index(short source)</h3>

功能：给策略添加指数行情服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_order">add_l2_order(short source)</h3>

功能：给策略添加下单行情服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_l2_trade">add_l2_trade(short source)</h3>

功能：给策略添加成交行情服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情接口常量|

<h3 id="add_register_td">add_register_td(short source)</h3>

功能：给策略添加交易服务

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|

<h3 id="register_bar_md">register_bar_md(short source, int min_interval, string start_time, string end_time)</h3>

功能：添加分钟线行情

|输入参数名|类型|参数说明|
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

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|输出参数名|类型|参数说明|
||byte|td状态|

<h3 id="set_pos">set_pos(PosHandlerPtr pos, short source)</h3>

功能：获取策略连接的交易接口状态

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|pos|PosHandlerPtr|持仓数据|
|source|short|行情交易接口常量|



<h3 id="get_ticker_pnl">get_ticker_pnl(short source, string ticker, bool include_fee=true)</h3>

功能：获取某合约或股票的持仓盈亏

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|ticker|string|合约或股票编号|
|include_fee|bool|是否包含交易费用|
|输出参数名|类型|参数说明|
||double|该合约或股票持仓盈亏|

<h3 id="get_effective_orders">get_effective_orders()</h3>

功能：获取有效（下单未全部成交）合约

|输出参数名|类型|参数说明|
|:--:|:--:|:--:|
||vector<int>|下单请求编号列表|




# util成员函数

<h3 id="insert_market_order">insert_market_order(short source, string instrument_id, string exchange_id, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下市价单

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|输出参数名|类型|参数说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_limit_order">insert_limit_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下限价单

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|输出参数名|类型|参数说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_fok_order">insert_fok_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下FOK类型订单（全部成交或则全部撤销），如果失败返回-1

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|输出参数名|类型|参数说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="insert_fak_order">insert_fak_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)</h3>

功能：下FAK类型订单（成交且为成交部分撤销），如果失败返回-1

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|instrument_id|string|合约代码|
|exchange_id|string|交易所代码|
|price|double|价格|
|volume|int|数量|
|direction|LfDirectionType|开平方向|
|offset|LfOffsetFlagType|多空方向|
|输出参数名|类型|参数说明|
||int|下单请求编号，若失败则返回-1|

<h3 id="req_position">req_position(short source)</h3>

功能：查询账户持仓

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|
|输出参数名|类型|参数说明|
||int|查询请求序号，若失败返回-1|

<h3 id="cancel_order">cancel_order(short source, int order_id)</h3>

功能：撤消指定合约

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|行情交易接口常量|
|order_id|int|要撤销的订单的下单请求编号|
|输出参数名|类型|参数说明|
|request_id|int|请求编号|

<h3 id="process_callback">process_callback(long cur_time)</h3>

功能：

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|cur_time|long|纳秒时间|
|输出参数名|类型|参数说明|
||int||

<h3 id="insert_callback">insert_callback(long nano, BLCallback& callback)</h3>

功能：添加回调函数

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|回调纳秒时间|
|callback| BLCallback&|回调函数|
|输出参数名|类型|参数说明|
||bool|回调函数是否添加成功|

<h3 id="insert_callback_py">insert_callback_py(long nano, boost::python::object func)</h3>

功能：添加回调函数（python）

|输入参数名|类型|回调|
|nano|nano|回调纳秒时间|
|func|boost::python::object|回调函数|
|输出参数名|类型|参数说明|
||bool|回调函数是否添加成功|

<h3 id="set_md_nano">set_md_nano(long cur_time)</h3>

功能：

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|cur_time|long|纳秒时间|

<h3 id="set_pos_back">set_pos_back(short source, const char* pos_str)</h3>

功能：设置策略持仓

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|source|short|交易接口常量|
|pos_str|char*|持仓信息字符串|

<h3 id="get_nano">get_nano()</h3>

功能：获取当前纳秒时间

|输出参数名|类型|参数说明|
|:--:|:--:|:--:|
||long|当前纳秒时间|

<h3 id="get_time">get_time()</h3>

功能：获取当前时间

|输出参数名|类型|参数说明|
|:--:|:--:|:--:|
||string|时间字符串|

<h3 id="parse_time">parse_time(string time_str)</h3>

功能：将字符串时间转换为纳秒时间

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|time_str|string|字符串时间|
|输出参数名|类型|参数说明|
||long|纳秒时间|

<h3 id="parse_nano">parse_nano(long nano)</h3>

功能：将纳秒时间转换为字符串时间

|输入参数名|类型|参数说明|
|:--:|:--:|:--:|
|nano|long|纳秒时间|
|输出参数名|类型|参数说明|
||string|字符串时间|


Version 版本
=============

* 0.0.1:
    初始化版本