#函数列表
* 回调函数 
 * [on_market_data](#on_market_data)行情回调
 * [on_market_bar](*on_market_bar)
 * [on_rtn_order](*on_rtn_order)
 * [on_rtn_trade](*on_rtn_trade)
 * [on_rsp_order](*on_rsp_order)
 * [on_rsp_position](*on_rsp_position)
 * [on_rsp_position](*on_rsp_position)
 * [on_switch_day](*on_switch_day)
 * [on_time](*on_time)
 * [on_td_login](*on_td_login)

* 功能函数
 * [init](*init)
 * [debug](*debug) 
 * [get_name](*get_name)

add_market_data(short source)
add_register_td(short source)
register_bar_md(short source, int min_interval, string start_time, string end_time)
run()
stop()
set_pos(PosHandlerPtr pos, short source)
PosHandlerPtr get_pos(short source)
byte get_td_status(short source) const

* data成员函数
 * [add_market_data](*add_market_data) 
 * [add_register_td](*add_register_td)
 * [register_bar_md](*register_bar_md)
 * [run](*run)
 * [stop](*stop)
 * [set_pos](*set_pos)
 * [get_td_status](*get_td_status)
 * [](*)
 * [](*)
 * [](*)

* util成语函数
 * [](*)
 * [](*)
 * [](*)
 * [](*)
 * [](*)
 * [](*)
 * [](*)
 * [](*)
 * [](*)
##回调函数

<h3 id="on_market_data">on_market_data(const LFMarketDataField* data, short source, long rcv_time)</h3>

<h3 id="on_market_bar">on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time)</h3>


<h3 id="on_rtn_order">on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time)</h3>


<h3 id="on_rtn_trade">on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time)</h3>


<h3 id="on_rsp_order">on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr)</h3>


<h3 id="on_rsp_position">on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time)</h3>


<h3 id="on_switch_day">on_switch_day(long rcv_time)</h3>


<h3 id="on_time">on_time(long cur_time)</h3>


<h3 id="on_td_login">on_td_login(bool ready, const json& js, short source)</h3>

##功能函数

<h3 id="debug">debug(const char* msg)</h3>

<h3 id="get_name">get_name()</h3>

<h3 id="init">init()</h3>



#data成员函数



#util成语函数

Version 版本
=============

* 0.0.1:
    初始化版本