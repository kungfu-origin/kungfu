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
 * DataConsumer: consume journal data.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Oct, 2017
 */

#ifndef WINGCHUN_DATACONSUMER_H
#define WINGCHUN_DATACONSUMER_H

#include "WC_DECLARE.h"
#include "JournalReader.h"
#include "PosHandler.hpp"
#include "longfist/LFDataStruct.h"

WC_NAMESPACE_START

struct StrategyUnit
{
    string name;
    string folder;
    long   last_switch_nano;
    int    rid_start;
    int    rid_end;
    yijinjing::PosHandlerPtr pos_handler;
};

class DataConsumer
{
protected:
    long start_nano;
    long end_nano;
    /** reader */
    yijinjing::JournalReaderPtr reader;
    /** current nano time */
    long cur_time;

protected:
    virtual void on_market_data(LFMarketDataField* md) {};
    virtual void on_switch_day(long nano_time) {};
    virtual void on_pos_set(json& j_pos, short source, long rcv_time, const string& name) {};
    /** strategy */
    virtual void on_strategy_start(json& j_start, long rcv_time, const string& name) {};
    virtual void on_strategy_end(json& j_end, long rcv_time, const string& name) {};
    virtual void on_td_connect(json& j_request, short source, long rcv_time, const string& name) {};
    virtual void on_td_ack(json& j_ack, short source, long rcv_time, const string& name) {};
    virtual void on_order_origin(LFInputOrderField* order, short source, int order_id, long rcv_time, long md_nano, const string& name) {};
    virtual void on_order_send(LFInputOrderField* order, short source, int order_id, long after_nano, long before_nano) {};
    virtual void on_rtn_order(LFRtnOrderField* order, short source, int order_id, long rcv_time) {};
    virtual void on_rtn_trade(LFRtnTradeField* trade, short source, int order_id, long rcv_time) {};
public:
    DataConsumer();
    void init(long start_nano, long end_nano);
    void run();
};

DECLARE_PTR(DataConsumer);

WC_NAMESPACE_END


#endif //WINGCHUN_DATACONSUMER_H
