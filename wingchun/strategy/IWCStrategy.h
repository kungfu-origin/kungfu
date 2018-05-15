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
 * IWCStrategy: base class of wingchun strategy.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#ifndef WINGCHUN_IWCSTRATEGY_H
#define WINGCHUN_IWCSTRATEGY_H

#include "WC_DECLARE.h"
#include "WCStrategyUtil.h"
#include "WCDataWrapper.h"
#include "KfLog.h"

WC_NAMESPACE_START

using yijinjing::KfLogPtr;

class IWCStrategy: public IWCDataProcessor
{
protected:
    bool td_is_ready(short source) const;
    bool td_is_connected(short source) const;
public:
    /** IWCDataProcessor functions *//* market data */
    virtual void on_market_data(const LFMarketDataField* data, short source, long rcv_time);
    virtual void on_market_data_level2(const LFL2MarketDataField* data, short source, long rcv_time);
    virtual void on_l2_index(const LFL2IndexField* data, short source, long rcv_time);
    virtual void on_l2_order(const LFL2OrderField* data, short source, long rcv_time);
    virtual void on_l2_trade(const LFL2TradeField* data, short source, long rcv_time);
    virtual void on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time);
    /* trade data */
    virtual void on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time);
    virtual void on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time);
    virtual void on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr);
    virtual void on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time);
    /* system utilities */
    virtual void on_switch_day(long rcv_time);
    virtual void on_time(long cur_time);
    virtual void on_td_login(bool ready, const json& js, short source);
    /* on log */
    virtual void debug(const char* msg) { KF_LOG_DEBUG(logger, msg); };
    /* get name */
    virtual string get_name() const { return name; };

    /**
     * setup data wrapper
     * by calling add_* functions in WCDataWrapper
     * also subscribe tickers
     */
    virtual void init() = 0;

public: // util functions, wrap upon WCStrategyUtil
    /** insert order, check status before calling WCStrategyUtil */
    int insert_market_order(short source, string instrument_id, string exchange_id, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_limit_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_fok_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_fak_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int req_position(short source);
    int cancel_order(short source, int order_id);

public:
    /** default destructor */
    virtual ~IWCStrategy();
    /** default constructor */
    IWCStrategy(const string& name);
    /* start data thread */
    virtual void start();
    /* run strategy in front end */
    void run();
    /* terminate data thread, should never be called within data thread */
    void terminate();
    /* stop send stop signal to data thread */
    void stop();
    /* block process by data thread */
    void block();

protected:
    /** logger, will be improved later */
    KfLogPtr logger;
    /** strategy name, not modifiable */
    const string name;
    /** strategy utils */
    WCStrategyUtilPtr util;
    /** data wrapper */
    WCDataWrapperPtr data;
    /** data thread */
    ThreadPtr data_thread;
};

WC_NAMESPACE_END

#endif //WINGCHUN_WCSTRATEGY_H
