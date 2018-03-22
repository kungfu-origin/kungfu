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
 * Strategy Util.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Wrap up all functions into StrategyUtil for wingchun strategy
 */

#ifndef YIJINJING_STRATEGY_UTIL_H
#define YIJINJING_STRATEGY_UTIL_H

#include "IStrategyUtil.h"
#include "JournalWriter.h"

YJJ_NAMESPACE_START

#define BL_BASE_FOLDER KUNGFU_JOURNAL_FOLDER "strategy/"
#define BL_REPLAY_FOLDER KUNGFU_JOURNAL_FOLDER "strategy_replay/"

FORWARD_DECLARE_PTR(StrategySocketHandler);
FORWARD_DECLARE_PTR(JournalWriter);
FORWARD_DECLARE_PTR(StrategyUtil);

/**
 * Wrapper of strategy utilities
 */
class StrategyUtil: public IStrategyUtil
{
protected:
    /** start rid, allocated by page service */
    int rid_start;
    /** end rid, allocated by page service */
    int rid_end;
    /** current rid */
    int cur_rid;

private:
    /** handler for socket stuff */
    StrategySocketHandlerPtr handler;
    /** wrap-up writer's features */
    JournalWriterPtr writer;

protected:
    /** initialization, register strategy to page engine */
    void init();

public:
    /** override IStrategyUtil */
    bool td_connect(short source);
    /** override IStrategyUtil */
    bool md_subscribe(const vector<string>& tickers, short source, short msg_type);
    /** override IStrategyUtil */
    bool register_strategy(int& requestIdStart, int& requestIdEnd);

    /** writer feature may used by strategy */
    long writeStr(const string& str);
    /** writer feature may used by strategy (python) */
    long writePyData(uintptr_t data, FH_TYPE_LENGTH length,
                     FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                     FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId);
    /** writer feature may used by strategy */
    long write_frame(const void* data, FH_TYPE_LENGTH length,
                     FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                     FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId);
    /** writer feature may used by strategy */
    long write_frame_extra(const void* data, FH_TYPE_LENGTH length,
                           FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                           FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                           FH_TYPE_NANOTM extraNano);

    /** subscribe md with MARKET_DATA flag */
    bool subscribeMarketData(const vector<string>& tickers, short source);
    /** subscribe md with L2_MD flag */
    bool subscribeL2MD(const vector<string>& tickers, short source);
    /** subscribe md with INDEX flag */
    bool subscribeIndex(const vector<string>& tickers, short source);
    /** subscribe md with ORDER_TRADE flag */
    bool subscribeOrderTrade(const vector<string>& tickers, short source);

    /** get current rid, sequentially incremental */
    int get_rid();
    /** get request id range */
    IntPair getRequestIds() const;
    /** get request id (python) */
    boost::python::tuple getPyRids() const;
    /** python interface for subscription */
    bool pySubscribe(boost::python::list tickers, int source);

public:
    /** destructor */
    ~StrategyUtil();
    /** constructor */
    StrategyUtil(const string& strategyName, bool isReplay=false);
    /** create method, only strategy name is enough
     *  folder and journal name is determined */
    static StrategyUtilPtr create(const string& strategyName, bool isReplay=false);
};

YJJ_NAMESPACE_END

#endif //YIJINJING_STRATEGY_UTIL_H
