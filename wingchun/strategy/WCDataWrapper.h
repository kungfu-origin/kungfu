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
 * WCStrategy: base class of wingchun strategy.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#ifndef WINGCHUN_WCDATAWRAPPER_H
#define WINGCHUN_WCDATAWRAPPER_H

#include "WC_DECLARE.h"
#include "JournalReader.h"
#include "IWCDataProcessor.h"

WC_NAMESPACE_START

using kungfu::yijinjing::JournalReaderPtr;
using kungfu::yijinjing::FeeHandlerPtr;
using kungfu::yijinjing::FeeHandler;

class WCStrategyUtil;

/** unknown, as default value */
#define CONNECT_TD_STATUS_UNKNOWN   0
/** td connect just added */
#define CONNECT_TD_STATUS_ADDED     1
/** td connect requested */
#define CONNECT_TD_STATUS_REQUESTED 2
/** got td ack msg, with valid pos */
#define CONNECT_TD_STATUS_ACK_POS   3
/** got td ack msg, no valid pos */
#define CONNECT_TD_STATUS_ACK_NONE  4
/** got td ack msg, set back */
#define CONNECT_TD_STATUS_SET_BACK  5

/** BarIntervalMaps: from min_interval -> BarMdMap */
typedef map<int, BarMdMap> BarIntervalMaps;
/** BarMdManager: from source -> BarIntervalMaps */
typedef map<short, BarIntervalMaps> BarMdManager;

class WCDataWrapper
{
protected:
    /** setup reader and request td connect */
    void pre_run();
    /** process position */
    void process_rsp_position(const LFRspPositionField* pos, bool is_last, int request_id, short source, long rcv_time);
    /** process ack of td */
    void process_td_ack(const string& content, short source, long rcv_time);
    /** set internal position */
    void set_internal_pos(PosHandlerPtr handler, short source);

protected:
    /** init bar */
    void init_bar(short source, int min_interval);
    /** send bar */
    void send_bar(short source, int min_interval);
    /** process bar */
    void process_bar(const LFMarketDataField* data, short source, long rcv_time);

public:
    /** default constructor */
    WCDataWrapper(IWCDataProcessor* processor,
                  WCStrategyUtil* util);

    void disable_auto_sub_mode() { auto_sub_mode_enabled = false; };
    /** add market data */
    void add_market_data(short source);
    /** register td */
    void add_register_td(short source);
    /**
     * register bar data
     * @param source
     * @param min_interval interval of seconds
     * @param start_time start from, format "09:00:00"
     * @param end_time end till, format "09:00:00"
     */
    void register_bar_md(short source, int min_interval, string start_time, string end_time);
    /** looping */
    void run();
    /** stop by changing force_stop flag */
    void stop();
    /** set current pos */
    void set_pos(PosHandlerPtr pos, short source);
    /** get pos, may create internally if not existing */
    PosHandlerPtr get_pos(short source);
    /** get td status */
    byte get_td_status(short source) const;
    /** get ticker status */
    double get_ticker_pnl(short source, string ticker, bool include_fee=true) const;
    /** get effective orders */
    vector<int> get_effective_orders() const;

protected:
    /** from request id to pos_handler, for each req_position */
    map<int, PosHandlerPtr> pos_handlers;
    /** from source to pos_handler, internal position */
    map<short, PosHandlerPtr> internal_pos;
    /** last price warehouse */
    umap<string, double> last_prices;
    /** request_id to order_status */
    umap<int, char> order_statuses;
    /** fee handler from td */
    map<short, FeeHandlerPtr> fee_handlers;

    BarMdManager        bars;
    vector<string>      folders;
    vector<string>      names;
    map<short, byte>    tds;
    JournalReaderPtr    reader;
    IWCDataProcessor*   processor;
    WCStrategyUtil*     util;
    int rid_start;
    int rid_end;
    bool force_stop;
    long cur_time;
    bool auto_sub_mode_enabled;
};

DECLARE_PTR(WCDataWrapper);

WC_NAMESPACE_END

#endif //WINGCHUN_WCDATAWRAPPER_H