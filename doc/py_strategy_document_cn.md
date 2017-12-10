函数索引
=======

* 回调函数

  * [initialize](#initialize)	              初始化函数 
  * on_bar	                  bar数据回调 
  * on_tick	                  ticker数据回调 
  * on_pos	                  持仓返回回调 
  * on_error	              错误回调 
  * on_rtn_order	          下单回报回调 
  * on_rtn_trade	          成交回报回调 
* 功能函数
  * context.stop	                停止策略
  * context.new_pos	                生成空持仓
  * context.get_nano	            获取系统当前纳秒时间
  * context.get_name	            获取策略名
  * context.parse_time	            时间字符串转纳秒时间
  * context.parse_nano	            纳秒时间转时间字符串
  * context.print_pos	            打印持仓
  * context.add_md	                添加行情接口
  * context.add_td	                添加交易接口
  * context.req_pos	                查询持仓
  * context.cancel_order	        撤销订单
  * context.set_pos	                设置策略持仓
  * context.get_pos	                获取策略持仓
  * context.subscribe	            订阅行情
  * context.register_bar	        订阅行情bar数据
  * context.insert_func_at	        在指定nano时间添加函数回调（无参数）
  * context.insert_func_after    	在当前时间后n秒添加回调（无参数）
  * context.insert_func_at_c	    在指定nano时间添加函数回调（context为参数）
  * context.insert_func_after_c	    在当前时间后n秒添加回调（context为参数）
  * context.insert_market_order	    下市价单
  * context.insert_limit_order	    下限价单
  * context.insert_fok_order	    下fok单
  * context.insert_fak_order    	下fak单
  * context.log_debug	            输入debug日志
  * context.log_info	            输出info日志
  * context.log_error	            输出error日志
  * context.log_fatal	            输出fatal日志

#持仓成员函数

PosHandler.add_pos	            添加持仓
PosHandler.set_pos	            设置持仓
PosHandler.dump	                持仓转为json格式规范的string
PosHandler.load	                load符合json的持仓字符串持仓
PosHandler.get_tickers	        获取有持仓的tickerlist
PosHandler.get_net_tot	        获取指定ticker的总净持仓
PosHandler.get_net_yd	        获取指定ticker的昨日净持仓
PosHandler.get_long_tot	        获取指定ticker的总多仓
PosHandler.get_long_yd	        获取指定ticker的昨日多仓
PosHandler.get_short_tot	    获取指定ticker的总空仓
PosHandler.get_short_yd	        获取指定ticker的昨日空仓

数据结构及常量索引
===============

LFMarketDataField	            行情数据结构
LFBarMarketDataField	        bar行情数据结构
LFRtnOrderField	                下单回报数据结构
LFRtnTradeField	                成交回报数据结构
SOURCE	                        接口常量
DIRECTION	                    买卖方向常量
OFFSET	                        开平方向常量
PRICE_TYPE	                    报单价格类型常量
ORDER_STATUS	                订单状态常量
POS_DIRECTION	                持仓方向常量
VOLUME_TYPE	                    订单交易量限制常量
TIME_CONDITION	                订单交易时间限制常量

函数定义
=======

<h3 id="initialize">initialize(context)</h3>
功能描述: 策略初始化的时候被触发，可以在其中添加行情和交易服务，以及完成其他需要在策略开始时初始化的操作   
context : 系统定义用于调用功能函数和存放任意数据的综合体

| 输入参数名 | 类型	| 参数说明 |
| :--------: | :-----------: | :-----------: |
| context | context | 系统定义用于调用功能函数和存放任意数据的综合体 |

<h3 id="on_bar">on_bar</h3>
功能描述: 当收到分钟线数据的时候函数触发

| 输入参数名 | 类型 | 参数说明 |
| :--------: | :-----------: | :-----------: |
| context | context | 系统定义用于调用功能函数和存放任意数据的综合体 |
| bars | dictionary((ticker(str) -> LFBarMarketDataField)) | bar结构list |
| souce | SOURCE | 行情交易接口常量 |
| rcv_time | long	返回纳秒时间 |


数据结构定义
==========

常量定义
=======


Version 版本
=============

* 0.0.1:
    初始化版本