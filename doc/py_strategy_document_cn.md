函数
=======

* 回调函数
  * [initialize](#initialize)初始化函数 
  * [on_bar](#on_bar)bar数据回调 
  * [on_tick](#on_tick)	 ticker数据回调 
  * [on_pos](#on_pos) 持仓返回回调 
  * [on_error](#on_error) 错误回调 
  * [on_rtn_order](#on_rtn_order)	 下单回报回调 
  * [on_rtn_trade](#on_rtn_trade)	 成交回报回调 
* 功能函数
  * [context.stop](#context.stop)	 停止策略
  * [context.new_pos](#context.new_pos) 生成空持仓
  * [context.get_nano](#context.get_nano) 获取系统当前纳秒时间
  * [context.get_name](#context.get_name) 获取策略名
  * [context.parse_time](#context.parse_time) 时间字符串转纳秒时间
  * [context.parse_nano](#context.parse_nano) 纳秒时间转时间字符串
  * [context.print_pos](#context.print_pos) 打印持仓
  * [context.add_md](#context.add_md) 添加行情接口
  * [context.add_td](#context.add_td) 添加交易接口
  * [context.req_pos](#context.req_pos) 查询持仓
  * [context.cancel_order](#context.cancel_order) 撤销订单
  * [context.set_pos](#context.set_pos) 设置策略持仓
  * [context.get_pos](#context.get_pos) 获取策略持仓
  * [context.subscribe](#context.subscribe) 订阅行情
  * [context.register_bar](#context.register_bar) 订阅行情bar数据
  * [context.insert_func_at](#context.insert_func_at) 在指定nano时间添加函数回调（无参数）
  * [context.insert_func_after](#context.insert_func_after) 在当前时间后n秒添加回调（无参数）
  * [context.insert_func_at_c](#context.insert_func_at_c) 在指定nano时间添加函数回调（context为参数）
  * [context.insert_func_after_c](#context.insert_func_after_c) 在当前时间后n秒添加回调（context为参数）
  * [context.insert_market_order](#context.insert_market_order) 下市价单
  * [context.insert_limit_order](#context.insert_limit_order) 下限价单
  * [context.insert_fok_order](#context.insert_fok_order) 下fok单
  * [context.insert_fak_order](#context.insert_fak_order) 下fak单
  * [context.log_debug](#context.log_debug) 输入debug日志
  * [context.log_info](#context.log_info) 输出info日志
  * [context.log_error](#context.log_error) 输出error日志
  * [context.log_fatal](#context.log_fatal) 输出fatal日志
* 持仓成员函数
  * [PosHandler.add_pos](#PosHandler.add_pos) 添加持仓
  * [PosHandler.set_pos](#PosHandler.set_pos) 设置持仓
  * [PosHandler.dump](#PosHandler.dump) 持仓转为json格式规范的string
  * [PosHandler.load](#PosHandler.load) load符合json的持仓字符串持仓
  * [PosHandler.get_tickers](#PosHandler.get_tickers) 获取有持仓的tickerlist
  * [PosHandler.get_net_tot](#PosHandler.get_net_tot) 获取指定ticker的总净持仓
  * [PosHandler.get_net_yd](#PosHandler.get_net_yd) 获取指定ticker的昨日净持仓
  * [PosHandler.get_long_tot](#PosHandler.get_long_tot) 获取指定ticker的总多仓
  * [PosHandler.get_long_yd](#PosHandler.get_long_yd) 获取指定ticker的昨日多仓
  * [PosHandler.get_short_tot](#PosHandler.get_short_tot) 获取指定ticker的总空仓
  * [PosHandler.get_short_yd](#PosHandler.get_short_yd) 获取指定ticker的昨日空仓

数据结构
===============
* [LFMarketDataField](#LFMarketDataField) 行情数据结构
* [LFBarMarketDataField](#LFBarMarketDataField) bar行情数据结构
* [LFRtnOrderField](#LFRtnOrderField)	下单回报数据结构
* [LFRtnTradeField](#LFRtnTradeField) 成交回报数据结构

常量
====
* [SOURCE](#SOURCE) 接口常量
* [DIRECTION](#DIRECTION) 买卖方向常量
* [OFFSET](#OFFSET) 买卖方向常量
* [PRICE_TYPE](#PRICE_TYPE) 报单价格类型常量
* [ORDER_STATUS](#ORDER_STATUS)	 订单状态常量
* [POS_DIRECTION](#POS_DIRECTION) 持仓方向常量
* [VOLUME_TYPE](#VOLUME_TYPE) 订单交易量限制常量
* [TIME_CONDITION](#TIME_CONDITION) 订单交易时间限制常量

函数定义
=======

### 回调函数

<h3 id="initialize">initialize(context)</h3>
功能描述: 策略初始化的时候被触发，可以在其中添加行情和交易服务，以及完成其他需要在策略开始时初始化的操作

context : 系统定义用于调用功能函数和存放任意数据的综合体

<h3 id="on_bar">on_bar</h3>
功能描述: 当收到分钟线数据的时候函数触发

参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

dictionary((ticker(str) -> [LFBarMarketDataField](#LFBarMarketDataField))) bar结构list

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间

<h3 id="on_tick">on_tick</h3>
功能描述: 收到tick数据时被触发

参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

[LFMarketDataField](#LFMarketDataField)	行情数据

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间

<h3 id="on_pos"> on_pos </h3>
功能描述: 当策略注册到TD时被触发一次，回报request_id为-1，如果策略首次运行，pos_handler为空值，如果策略之前运行且设置过持仓，pos_handler为策略持仓。同时该函数在该策略查询了持仓以后会被触发，request_id为查询持仓时的请求序号，pos_handler为账户持仓

参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

PosHandler	持仓数据

int	请求编号

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间

<h3 id="on_error"> on_error </h3>
功能描述:当收到错误回报时被触发

参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

int	错误ID

string	错误信息

int	请求编号

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间 


<h3 id="on_rtn_order"> on_rtn_order </h3>
功能描述: 收到策略下单的下单回报时被触发，非本策略下单的下单回报不会触发该函数
参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

[LFRtnOrderField](#LFRtnOrderField)	下单返回数据结构

int	下单编号

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间


<h3 id="on_rtn_trade"> on_rtn_trade </h3>
功能描述: 收到策略下单的成交回报时被触发，非本策略下单的成交回报不会触发该函数
参数数据类型及解释：

context	系统定义用于调用功能函数和存放任意数据的综合体

[LFRtnTradeField](#LFRtnTradeField)	成交返回数据结构

int	下单编号

[SOURCE](#SOURCE)	行情交易接口常量

long	返回纳秒时间

<h3 id="context.stop">context.stop</h3>
功能描述: 停止策略（但不是立即停止，可能会在真正停止之前等待几秒）

参数数据类型及解释：

返回值类型及解释：

<h3 id="context.new_pos">context.new_pos</h3>
功能描述:创建一个空的持仓 

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

PosHandler	持仓数据结构(空持仓)

<h3 id="context.get_nano">context.get_nano</h3>
功能描述: 获得当前纳秒时间

参数数据类型及解释：

返回值类型及解释：

long	系统当前纳秒时间

<h3 id="context.get_name">context.get_name</h3>
功能描述: 获取策略名

参数数据类型及解释：

返回值类型及解释：

string	策略名称

<h3 id="context.parse_time">context.parse_time</h3>
功能描述: 将时间字符串转换为纳秒时间 

参数数据类型及解释：

string	时间字符串("%Y-%m-%d %H:%M:%S")

返回值类型及解释：

long	纳秒时间

<h3 id="context.parse_nano">context.parse_nano</h3>
功能描述: 将纳秒时间转换为字符串时间

参数数据类型及解释：

long	纳秒时间

返回值类型及解释：

string	时间字符串("%Y-%m-%d %H:%M:%S")

<h3 id="context.print_pos">context.print_pos</h3>
功能描述: 打印持仓

参数数据类型及解释：

返回值类型及解释：

PosHandler	持仓数据

<h3 id="context.add_md">context.add_md</h3>
功能描述: 给策略添加行情服务

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

<h3 id="context.add_td">context.add_td</h3>
功能描述: 给策略添加交易服务

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

<h3 id="context.req_pos">context.req_pos</h3>
功能描述: 查询账户持仓，如果查询失败返回-1，查询成功则返回request_id，当收到交易所返回持仓时通过回调on_pos传回持仓

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

int	请求编号

<h3 id="context.cancel_order">context.cancel_order</h3>
功能描述: 使用下单编号撤销订单，如果失败则返回-1

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

int	下单编号

返回值类型及解释：

int	请求编号

<h3 id="context.set_pos">context.set_pos</h3>
功能描述: 设置策略持仓。

参数数据类型及解释：

PosHandler	持仓数据结构

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

<h3 id="context.get_pos">context.get_pos</h3>
功能描述:获得策略持仓 

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

PosHandler	持仓数据结构

<h3 id="context.subscribe">context.subscribe</h3>
功能描述: 订阅行情

参数数据类型及解释：

list 订阅合约列表

[SOURCE](#SOURCE)	行情交易接口常量

返回值类型及解释：

<h3 id="context.register_bar">context.register_bar</h3>
功能描述: 订阅分钟线行情，间隔时间可为自定义整数

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

int	bar间隔分钟数

string	bar数据开始时间

string	bar数据结束时间

返回值类型及解释：


<h3 id="context.insert_func_at">context.context.insert_func_at</h3>
功能描述: 在指定纳秒时间添加回调函数

参数数据类型及解释：

long	回调纳秒时间

func	回调函数

返回值类型及解释：


<h3 id="context.insert_func_at_c">context.insert_func_at_c</h3>
功能描述: 在指定纳秒时间添加以context为参数的回调函数

参数数据类型及解释：

long	回调纳秒时间

func	回调函数（以context为默认参数）

返回值类型及解释：

<h3 id="context.insert_func_after">context.insert_func_after</h3>
功能描述: 在指定秒数后添加回调函数

参数数据类型及解释：

int	需要在多少秒之后回调

func	回调函数

返回值类型及解释：

<h3 id="context.insert_func_after_c">context.insert_func_after_c</h3>
功能描述: 在指定秒数后添加以context为参数的回调函数

参数数据类型及解释：

int	需要在多少秒以后回调

func	回调函数（以context为参数）

返回值类型及解释：

<h3 id="context.insert_market_order">context.insert_market_order</h3>
功能描述: 下市价单，如果失败返回-1

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

string	合约代码

string	交易所代码

int	数量

[DIRECTION](#DIRECTION)	买卖方向

[OFFSET](#OFFSET)	开平方向

返回值类型及解释：

int	下单编号

<h3 id="context.insert_limit_order">context.insert_limit_order</h3>
功能描述: 下限价单，如果失败返回-1

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

string	合约代码

string	交易所代码

float	价格

int	数量

[DIRECTION](#DIRECTION)	买卖方向

[OFFSET](#OFFSET)	开平方向

返回值类型及解释：

int	下单编号

<h3 id="context.insert_fok_order">context.insert_fok_order</h3>
功能描述: 下FOK类型订单（全部成交或则全部撤销），如果失败返回-1

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

string	合约代码

string	交易所代码

float	价格

int	数量

[DIRECTION](#DIRECTION)	买卖方向

[OFFSET](#OFFSET)	开平方向

返回值类型及解释：

int	下单编号

<h3 id="context.insert_fak_order">context.insert_fak_order</h3>
功能描述: 下FAK类型订单（成交且为成交部分撤销），如果失败返回-1

参数数据类型及解释：

[SOURCE](#SOURCE)	行情交易接口常量

string	合约代码

string	交易所代码

float	价格

int	数量

[DIRECTION](#DIRECTION)	买卖方向

[OFFSET](#OFFSET)	开平方向

返回值类型及解释：

int	下单编号

<h3 id="context.log_debug">context.log_debug</h3>
功能描述: 输出debug日志

参数数据类型及解释：

string	日志字符串

返回值类型及解释：

<h3 id="context.log_info">context.log_info</h3>
功能描述: 输出info日志

参数数据类型及解释：

string	日志字符串

返回值类型及解释：

<h3 id="context.log_error">context.log_error</h3>
功能描述: 输出错误日志

参数数据类型及解释：

string	日志字符串

返回值类型及解释：

<h3 id="context.log_fatal">context.log_fatal</h3>
功能描述: 输出fatal日志

参数数据类型及解释：

string	日志字符串

返回值类型及解释：

<h3 id="PosHandler.add_pos">PosHandler.add_pos</h3>
功能描述: 向持仓数据项中添加持仓增量

参数数据类型及解释：

string	需要增加仓位的合约号

[POS_DIRECTION](#POS_DIRECTION)	仓位方向

int	总仓位

int	昨仓位

返回值类型及解释：


<h3 id="PosHandler.set_pos">PosHandler.set_pos</h3>
功能描述: 向持仓数据项中添加持仓（会替换原有项）

参数数据类型及解释：

string	需要增加仓位的合约号

[POS_DIRECTION](#POS_DIRECTION)	仓位方向

int	总仓位

int	昨仓位

返回值类型及解释：

<h3 id="PosHandler.dump">PosHandler.dump</h3>
功能描述: 持仓转换为json格式字符串

参数数据类型及解释：

返回值类型及解释：

string	json标准格式的持仓字符串

<h3 id="PosHandler.load">PosHandler.load</h3>
功能描述: 将json格式的持仓数据设置为持仓

参数数据类型及解释：

string	json标准格式的持仓字符串

返回值类型及解释：


<h3 id="PosHandler.get_tickers">PosHandler.get_tickers</h3>
功能描述: 获得持仓数据的合约列表

参数数据类型及解释：

返回值类型及解释：

list	有持仓的合约list

<h3 id="PosHandler.get_net_tot">PosHandler.get_net_tot</h3>
功能描述: 获取某只股票总持仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约总净持仓

<h3 id="PosHandler.get_net_yd">PosHandler.get_net_yd</h3>
功能描述: 获取某只股票昨仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约昨日净持仓

<h3 id="PosHandler.get_long_tot">PosHandler.get_long_tot</h3>
功能描述: 获取期货某合约做多总持仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约总多仓

<h3 id="PosHandler.get_long_yd">PosHandler.get_long_yd</h3>
功能描述: 获取期货某合约做多昨仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约昨多仓

<h3 id="PosHandler.get_short_tot">PosHandler.get_short_tot</h3>
功能描述: 获取期货某合约做空总持仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约总空仓

<h3 id="PosHandler.get_short_yd">PosHandler.get_short_yd</h3>
功能描述: 获取期货某合约做空昨仓

参数数据类型及解释：

string	要获取仓位的合约

返回值类型及解释：

int	指定合约昨空仓


	
数据结构定义
==========

<h3 id="LFMarketDataField">LFMarketDataField</h3>
内容: 行情数据结构

成员、类型与解释：

TradingDay	string	交易日

InstringumentID	string	合约代码

ExchangeID	string	交易所代码

ExchangeInstID	string	合约在交易所的代码

LastPrice	float	最新价

PreSettlementPrice	float	上次结算价

PreClosePrice	float	昨收盘

PreOpenInterest	float	昨持仓量

OpenPrice	float	今开盘

HighestPrice	float	最高价

LowestPrice	float	最低价

Volume	int	数量

Turnover	float	成交金额

OpenInterest	float	持仓量

ClosePrice	float	今收盘

SettlementPrice	float	本次结算价

UpperLimitPrice	float	涨停板价

LowerLimitPrice	float	跌停板价

PreDelta	float	昨虚实度

CurrDelta	float	今虚实度

UpdateTime	string	最后修改时间

UpdateMillisec	int	最后修改毫秒

BidPrice1	float	申买价一

BidVolume1	int	申买量一

AskPrice1	float	申卖价一

AskVolume1	int	申卖量一

BidPrice2	float	申买价二

BidVolume2	int	申买量二

AskPrice2	float	申卖价二

AskVolume2	int	申卖量二

BidPrice3	float	申买价三

BidVolume3	int	申买量三

AskPrice3	float	申卖价三

AskVolume3	int	申卖量三

BidPrice4	float	申买价四

BidVolume4	int	申买量四

AskPrice4	float	申卖价四

AskVolume4	int	申卖量四

BidPrice5	float	申买价五

BidVolume5	int	申买量五

AskPrice5	float	申卖价五

AskVolume5	int	申卖量五

<h3 id="LFBarMarketDataField">LFBarMarketDataField</h3>
内容: 分钟线数据结构

成员、类型与解释：

TradingDay	string	交易日

InstringumentID	string	合约代码

UpperLimitPrice	float	涨停板价

LowerLimitPrice	float	跌停板价

StartUpdateTime	string	首tick修改时间

StartUpdateMillisec	int	首tick最后修改毫秒

EndUpdateTime	string	尾tick最后修改时间

EndUpdateMillisec	int	尾tick最后修改毫秒

Open	float	开

Close	float	收

Low	float	最高价

High	float	最低价

Volume	float	区间交易量

StartVolume	float	初始总交易量

<h3 id="LFRtnOrderField">LFRtnOrderField</h3>
内容: 下单回报数据结构

成员、类型与解释：

BrokerID	string	经纪公司代码

ParticipantID	string	会员代码

InvestorID	string	投资者代码

BusinessUnit	string	业务单元

InstringumentID	string	合约代码

OrderRef	string	报单引用

ExchangeID	string	交易所代码

LimitPrice	float	价格

VolumeTraded	int	今成交数量

VolumeTotal	int	剩余数量

VolumeTotalOriginal	int	数量

TimeCondition	[TIME_CONDITION](#TIME_CONDITION)	有效期类型

VolumeCondition	[VOLUME_TYPE](#VOLUME_TYPE)	成交量类型

OrderPriceType	[PRICE_TYPE](#PRICE_TYPE)	报单价格条件

Direction	DIRECTION	买卖方向

OffsetFlag	[OFFSET](#OFFSET)	开平标志

OrderStatus	[ORDER_STATUS](#ORDER_STATUS)	报单状态

<h3 id="LFRtnTradeField">LFRtnTradeField</h3>
内容: 成交回报数据结构

成员、类型与解释：

BrokerID	string	经纪公司代码

InvestorID	string	投资者代码

BusinessUnit	string	业务单元

InstringumentID	string	合约代码

OrderRef	string	报单引用

ExchangeID	string	交易所代码

Price	float	价格

Volume	int	数量

TradingDay	string	交易日

TradeTime	string	成交时间

Direction	DIRECTION	买卖方向

OffsetFlag	[OFFSET](#OFFSET)	开平标志


常量定义
=======

<h3 id="[SOURCE]">SOURCE</h3>
内容: 接口常量

标记、值与含义：

CTP	 1 

<h3 id="DIRECTION"> DIRECTION </h3>
功能描述: 买卖方向常量

标记、值与含义：

Buy	‘1'	买

Sell	‘2'	卖

<h3 id="OFFSET"> OFFSET </h3>
功能描述: 买卖方向常量

成员、类型与解释：

Open	‘1'	开仓

Close	‘2'	平仓

CloseToday	‘3'	平今

CloseYesterday	‘4'	平昨

<h3 id="PRICE_TYPE"> PRICE_TYPE </h3>
功能描述: 报单价格类型常量

标记、值与含义：

AnyPrice	‘1'	任意价

LimitPrice	‘2'	限价

BestPrice	‘3'	最优价

<h3 id="ORDER_STATUS"> ORDER_STATUS </h3>
功能描述: 订单状态常量

标记、值与含义：

AllTraded	‘0'	全部成交（最终状态）

PartTradedQueueing	‘1'	部分成交还在队列中

PartTradedNotQueueing	‘2'	部分成交不在队列中（部成部撤，最终状态）

NoTradeQueueing	‘3'	未成交还在队列中

NoTradeNotQueueing	‘4'	未成交不在队列中（被拒绝，最终状态）

Canceled	‘5'	撤单

Unknown	‘a'	未知

<h3 id="POS_DIRECTION"> POS_DIRECTION </h3>
功能描述: 持仓方向常量

标记、值与含义：

Net	‘1'	净

Long	‘2'	多头

Short	‘3'	空头

<h3 id="VOLUME_TYPE"> VOLUME_TYPE </h3>
功能描述: 订单交易量限制常量

标记、值与含义：

AV	‘1'	任何数量

MV	‘2'	最小数量

CV	‘3'	全部数量

<h3 id="TIME_CONDITION"> TIME_CONDITION </h3>
功能描述: 订单交易时间限制常量

标记、值与含义：

IOC	‘1'	立即完成，否则撤销

GFS	‘2'	本节有效

GFD	‘3'	当日有效

GTD	‘4'	指定日期前有效

GTC	‘5'	撤销前有效

GFA	‘6'	集合竞价有效


	
Version 版本
=============

* 0.0.1:
    初始化版本