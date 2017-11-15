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
 * IMDEngine: base class of all market data engine.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   April, 2017
 */

#ifndef WINGCHUN_IMDENGINE_H
#define WINGCHUN_IMDENGINE_H

#include "IEngine.h"
#include "longfist/LFDataStruct.h"

WC_NAMESPACE_START

typedef std::map<string, int> SubCountMap;

/**
 * Base class of all market data engine
 */
class IMDEngine: public IEngine
{
private:
    /** internal structure for subscription */
    vector<string> subs_tickers;
    /** internal structure for subscription */
    vector<string> subs_markets;
    /** internal structure for auto subscription when re-begin md engine */
    std::map<short, SubCountMap> history_subs; // { msg_type: { ticker@market : sub_count } }

protected:
    /** default constructor */
    IMDEngine(short source);
    /** setup reader thread */
    virtual void set_reader_thread();
    /** pre-run after login */
    virtual void pre_run();
    /** keep listening to system journal */
    void listening();
    /** init reader and writer */
    virtual void init();
    /** subscribe historically-subscribed tickers */
    void subscribeHistorySubs();
public:
    /** subscribe market data, should be override by child-class */
    virtual void subscribeMarketData(const vector<string>& instruments, const vector<string>& markets)
    {
        KF_LOG_ERROR(logger, "subscribe market data not supported here!");
    }
    /** subscribe l2 data, should be override by child-class */
    virtual void subscribeL2MD(const vector<string>& instruments, const vector<string>& markets)
    {
        KF_LOG_ERROR(logger, "subscribe l2 data not supported here!");
    }
    /** subscribe index data, should be override by child-class */
    virtual void subscribeIndex(const vector<string>& instruments, const vector<string>& markets)
    {
        KF_LOG_ERROR(logger, "subscribe index data not supported here!");
    }
    /** subscribe order and trade data, should be override by child-class */
    virtual void subscribeOrderTrade(const vector<string>& instruments, const vector<string>& markets)
    {
        KF_LOG_ERROR(logger, "subscribe order trade data not supported here!");
    }
    /** on market data, engine (on_data) */
    void on_market_data(const LFMarketDataField* data);
};

DECLARE_PTR(IMDEngine);

WC_NAMESPACE_END

#endif //WINGCHUN_IMDENGINE_H
