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

#include "StrategyUtil.h"
#include "StrategySocketHandler.h"
#include "TypeConvert.hpp"
#include "PageCommStruct.h"
#include "longfist/sys_messages.h"

USING_YJJ_NAMESPACE

StrategyUtil::StrategyUtil(const string& strategyName, bool isReplay)
{
    if (isReplay)
    {
        throw std::runtime_error("replay mode is not fully supported yet!");
        writer = JournalWriter::create(BL_REPLAY_FOLDER, strategyName, handler);
    }
    else
    {
        handler = StrategySocketHandlerPtr(new StrategySocketHandler(strategyName));
        writer = JournalWriter::create(BL_BASE_FOLDER, strategyName, handler);
        init();
    }
}

StrategyUtilPtr StrategyUtil::create(const string& strategyName, bool isReplay)
{
#ifndef USE_PAGED_SERVICE
    throw std::runtime_error("strategy utilities cannot be utilized without paged");
#endif
    StrategyUtilPtr util = StrategyUtilPtr(new StrategyUtil(strategyName, isReplay));
    return util;
}

StrategyUtil::~StrategyUtil()
{
    writer.reset();
    handler.reset();
}

void StrategyUtil::init()
{
    register_strategy(rid_start, rid_end);
    cur_rid = 0;
}

bool StrategyUtil::td_connect(short source)
{
    if (handler.get() != nullptr)
        return handler->td_connect(source);
    else
        return false;
}

IntPair StrategyUtil::getRequestIds() const
{
    return std::make_pair(rid_start, rid_end);
}

boost::python::tuple StrategyUtil::getPyRids() const
{
    return boost::python::make_tuple(rid_start, rid_end);
}

bool StrategyUtil::pySubscribe(boost::python::list pyTickers, int source)
{
    vector<string> tickers = py_list_to_std_vector<string>(pyTickers);
    return subscribeMarketData(tickers, source);
}

bool StrategyUtil::md_subscribe(const vector<string>& tickers, short source, short msg_type)
{
    if (handler.get() != nullptr)
        return handler->md_subscribe(tickers, source, msg_type);
    else
        return false;
}

bool StrategyUtil::register_strategy(int& requestIdStart, int& requestIdEnd)
{
    if (handler.get() != nullptr)
        return handler->register_strategy(requestIdStart, requestIdEnd);
    else
        return false;
}

long StrategyUtil::writeStr(const string& str)
{
    return writer->writeStr(str);
}

long StrategyUtil::write_frame(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                               FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId)
{
    return writer->write_frame(data, length, source, msgType, lastFlag, requestId);
}

long StrategyUtil::write_frame_extra(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                     FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId, FH_TYPE_NANOTM extraNano)
{
    return writer->write_frame_extra(data, length, source, msgType, lastFlag, requestId, extraNano);
}

long StrategyUtil::writePyData(uintptr_t data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                               FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId)
{
    return writer->writePyData(data, length, source, msgType, lastFlag, requestId);
}

bool StrategyUtil::subscribeMarketData(const vector<string>& tickers, short source)
{
    return md_subscribe(tickers, source, MSG_TYPE_SUBSCRIBE_MARKET_DATA);
}

bool StrategyUtil::subscribeL2MD(const vector<string>& tickers, short source)
{
    return md_subscribe(tickers, source, MSG_TYPE_SUBSCRIBE_L2_MD);
}

bool StrategyUtil::subscribeIndex(const vector<string>& tickers, short source)
{
    return md_subscribe(tickers, source, MSG_TYPE_SUBSCRIBE_INDEX);
}

bool StrategyUtil::subscribeOrderTrade(const vector<string>& tickers, short source)
{
    return md_subscribe(tickers, source, MSG_TYPE_SUBSCRIBE_ORDER_TRADE);
}

int StrategyUtil::get_rid()
{
    return (cur_rid ++ % REQUEST_ID_RANGE) + rid_start;
}