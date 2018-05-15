/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * StrategyDataConsumer: strategy-centralized data consumer.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Oct, 2017
 */

#ifndef WINGCHUN_STRATEGYDATACONSUMER_H
#define WINGCHUN_STRATEGYDATACONSUMER_H

#include "DataConsumer.h"
#include "longfist/LFDataStruct.h"

WC_NAMESPACE_START

struct OrderLatencyUnit
{
    int     order_id;
    string  ticker;
    double  price;
    int     volume;
    char    direction;
    char    offset;
    // to update
    char    order_status;
    // proceeding
    int     number_rtn_order;
    int     number_rtn_trade;
    // time
    long    md_time;
    long    oe_time;
    long    send_before_time;
    long    send_after_time;
    long    rsp_time;
    long    first_rtn_order_time;
    long    first_rtn_trade_time;

    OrderLatencyUnit(): order_id(0),
                        price(0.0),
                        volume(0),
                        number_rtn_order(0),
                        number_rtn_trade(0),
                        md_time(-1),
                        oe_time(-1),
                        send_before_time(-1),
                        send_after_time(-1),
                        rsp_time(-1),
                        first_rtn_order_time(-1),
                        first_rtn_trade_time(-1){};
};

class StrategyBasicDataConsumer: public DataConsumer
{
protected:
    StrategyUnit unit;
    int cur_session;
    int tar_session;
    string strategy_name;
protected:
    virtual void on_strategy_start(json& j_start, long rcv_time, const string& name);
    virtual void on_strategy_end(json& j_end, long rcv_time, const string& name);
public:
    StrategyBasicDataConsumer(const string& strategy_name, int target_session_id=-1);
};

class StrategyDataConsumer: public StrategyBasicDataConsumer
{
protected:
    /** order latency information map */
    map<int, OrderLatencyUnit> order_map;
    /** map[order_id-> map[rcv_time-> RTN_TRADE] ] */
    map<int, map<long, LFRtnTradeField> > rtn_trade_map;

protected:
    virtual void on_pos_set(json& j_pos, short source, long rcv_time, const string& name);
    virtual void on_td_connect(json& j_request, short source, long rcv_time, const string& name);
    virtual void on_td_ack(json& j_ack, short source, long rcv_time, const string& name);

    virtual void on_order_origin(LFInputOrderField* order, short source, int order_id, long rcv_time, long md_nano, const string& name);
    virtual void on_order_send(LFInputOrderField* order, short source, int order_id, long after_nano, long before_nano);
    virtual void on_rtn_order(LFRtnOrderField* order, short source, int order_id, long rcv_time);
    virtual void on_rtn_trade(LFRtnTradeField* trade, short source, int order_id, long rcv_time);

public:
    StrategyDataConsumer(const string& strategy_name, int target_session_id);

    void print_stat() const;
    void print_order() const;
    void print_trade() const;
    void print_detail() const;
};

WC_NAMESPACE_END

#endif //WINGCHUN_STRATEGYDATACONSUMER_H
