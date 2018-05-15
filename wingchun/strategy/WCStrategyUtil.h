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
 * WCStrategyUtil: utility functions for strategy.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#ifndef WINGCHUN_WCSTRATEGYUTIL_H
#define WINGCHUN_WCSTRATEGYUTIL_H

#include "WC_DECLARE.h"
#include "StrategyUtil.h"
#include "longfist/LFConstants.h"
#include <queue>
#include <boost/function.hpp>

WC_NAMESPACE_START

using kungfu::yijinjing::StrategyUtil;

typedef boost::function<void ()> BLCallback;

struct BLCallbackUnit
{
    long nano;
    BLCallback func;
    BLCallbackUnit(): nano() {}
    BLCallbackUnit(long nano, const BLCallback& func): nano(nano), func(func) {}
    bool operator() (BLCallbackUnit& i, BLCallbackUnit& j) { return i.nano > j.nano; }
};

typedef std::priority_queue<BLCallbackUnit, vector<BLCallbackUnit>, BLCallbackUnit> BLCallbackMinHeap;

#define WC_STRATEGY_TD_FAILED       0
#define WC_STRATEGY_TD_ACK          1
#define WC_STRATEGY_TD_READY        2
/**
 * utilities includes:
 *  1. all journal writing (insert order, req pos, etc)
 *  2. request id related
 *  3. call back insert / process
 */
class WCStrategyUtil: public StrategyUtil
{
private:
    long md_nano;
    long cur_nano;
    string strategy_name;
    BLCallbackMinHeap callback_heap;

public:
    WCStrategyUtil(const string& strategyName);

    /** subscribe */
    /** subscribe md with MARKET_DATA flag */
    bool subscribe_market_data(boost::python::list tickers, short source);

    /** take actions */
    int insert_market_order(short source, string instrument_id, string exchange_id, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_limit_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_fok_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int insert_fak_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset);
    int req_position(short source);
    int cancel_order(short source, int order_id);

    /** callback */
    /** process callback functions that match time, return number of functions that executed */
    int process_callback(long cur_time);
    /** insert callback functions */
    bool insert_callback(long nano, BLCallback& callback);
    /** insert python callback functions */
    bool insert_callback_py(long nano, boost::python::object func);
    /** set md nano time */
    void set_md_nano(long cur_time) { md_nano = cur_time; };
    /** set pos back */
    void set_pos_back(short source, const char* pos_str);

    /** basic utilities */
    /** nano time */
    long   get_nano();
    /** get string time with format "%Y-%m-%d %H:%M:%S" */
    string get_time();
    /** parse time */
    long   parse_time(string time_str);
    /** parse nano */
    string parse_nano(long nano);
};

DECLARE_PTR(WCStrategyUtil);

WC_NAMESPACE_END

#endif //WINGCHUN_WCSTRATEGYUTIL_H
