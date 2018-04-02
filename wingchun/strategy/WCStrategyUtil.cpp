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

#include "WCStrategyUtil.h"
#include "IWCDataProcessor.h"
#include "TypeConvert.hpp"
#include "Timer.h"
#include "longfist/LFDataStruct.h"
#include "longfist/sys_messages.h"

USING_WC_NAMESPACE

WCStrategyUtil::WCStrategyUtil(const string& strategyName):
    StrategyUtil(strategyName, false), strategy_name(strategyName)
{
    cur_nano = 0;
    md_nano = 0;
}

/** subscribe md with MARKET_DATA flag */
bool WCStrategyUtil::subscribe_market_data(boost::python::list tickers, short source)
{
    vector<string> vec_ticker = kungfu::yijinjing::py_list_to_std_vector<string>(tickers);
    return subscribeMarketData(vec_ticker, source);
}

int WCStrategyUtil::process_callback(long cur_time)
{
    cur_nano = cur_time;
    int count = 0;
    while (!callback_heap.empty())
    {
        auto callbackUnit = callback_heap.top();
        if (callbackUnit.nano <= cur_nano)
        {
            if (IWCDataProcessor::signal_received <= 0)
            {
                START_PYTHON_FUNC_CALLING
                callbackUnit.func();
                END_PYTHON_FUNC_CALLING
            }
            callback_heap.pop();
            count ++;
        }
        else
        {
            break;
        }
    }
    return count;
}

bool WCStrategyUtil::insert_callback(long nano, BLCallback& callback)
{
    if (nano > cur_nano)
    {
        callback_heap.push(BLCallbackUnit(nano, callback));
        return true;
    }
    return false;
}

bool WCStrategyUtil::insert_callback_py(long nano, boost::python::object func)
{
    BLCallback callback = static_cast<BLCallback>(func);
    return insert_callback(nano, callback);
}

long WCStrategyUtil::get_nano()
{
    return kungfu::yijinjing::getNanoTime();
}

const char time_format[] = "%Y-%m-%d %H:%M:%S";

string WCStrategyUtil::get_time()
{
    return kungfu::yijinjing::parseNano(kungfu::yijinjing::getNanoTime(), time_format);
}

long WCStrategyUtil::parse_time(string time_str)
{
    return kungfu::yijinjing::parseTime(time_str.c_str(), time_format);
}

string WCStrategyUtil::parse_nano(long nano)
{
    return kungfu::yijinjing::parseNano(nano, time_format);
}

int WCStrategyUtil::insert_market_order(short source,
                                        string instrument_id,
                                        string exchange_id,
                                        int volume,
                                        LfDirectionType direction,
                                        LfOffsetFlagType offset)
{
    int rid = get_rid();
    LFInputOrderField order = {};
    strcpy(order.ExchangeID, exchange_id.c_str());
    strcpy(order.InstrumentID, instrument_id.c_str());
    order.LimitPrice = 0;
    order.Volume = volume;
    order.MinVolume = 1;
    order.TimeCondition = LF_CHAR_IOC;
    order.VolumeCondition = LF_CHAR_AV;
    order.OrderPriceType = LF_CHAR_AnyPrice;
    order.Direction = direction;
    order.OffsetFlag = offset;
    order.HedgeFlag = LF_CHAR_Speculation;
    order.ForceCloseReason = LF_CHAR_NotForceClose;
    order.StopPrice = 0;
    order.IsAutoSuspend = true;
    order.ContingentCondition = LF_CHAR_Immediately;
    strcpy(order.MiscInfo, strategy_name.c_str());
    write_frame_extra(&order, sizeof(LFInputOrderField), source, MSG_TYPE_LF_ORDER, 1/*lastflag*/, rid, md_nano);
    return rid;
}

int WCStrategyUtil::insert_limit_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)
{
    int rid = get_rid();
    LFInputOrderField order = {};
    strcpy(order.ExchangeID, exchange_id.c_str());
    strcpy(order.InstrumentID, instrument_id.c_str());
    order.LimitPrice = price;
    order.Volume = volume;
    order.MinVolume = 1;
    order.TimeCondition = LF_CHAR_GFD;
    order.VolumeCondition = LF_CHAR_AV;
    order.OrderPriceType = LF_CHAR_LimitPrice;
    order.Direction = direction;
    order.OffsetFlag = offset;
    order.HedgeFlag = LF_CHAR_Speculation;
    order.ForceCloseReason = LF_CHAR_NotForceClose;
    order.StopPrice = 0;
    order.IsAutoSuspend = true;
    order.ContingentCondition = LF_CHAR_Immediately;
    strcpy(order.MiscInfo, strategy_name.c_str());
    write_frame_extra(&order, sizeof(LFInputOrderField), source, MSG_TYPE_LF_ORDER, 1/*lastflag*/, rid, md_nano);
    return rid;
}

int WCStrategyUtil::insert_fok_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)
{
    int rid = get_rid();
    LFInputOrderField order = {};
    strcpy(order.ExchangeID, exchange_id.c_str());
    strcpy(order.InstrumentID, instrument_id.c_str());
    order.LimitPrice = price;
    order.Volume = volume;
    order.MinVolume = 1;
    order.TimeCondition = LF_CHAR_IOC;
    order.VolumeCondition = LF_CHAR_CV;
    order.OrderPriceType = LF_CHAR_LimitPrice;
    order.Direction = direction;
    order.OffsetFlag = offset;
    order.HedgeFlag = LF_CHAR_Speculation;
    order.ForceCloseReason = LF_CHAR_NotForceClose;
    order.StopPrice = 0;
    order.IsAutoSuspend = true;
    order.ContingentCondition = LF_CHAR_Immediately;
    strcpy(order.MiscInfo, strategy_name.c_str());
    write_frame_extra(&order, sizeof(LFInputOrderField), source, MSG_TYPE_LF_ORDER, 1/*lastflag*/, rid, md_nano);
    return rid;
}

int WCStrategyUtil::insert_fak_order(short source, string instrument_id, string exchange_id, double price, int volume, LfDirectionType direction, LfOffsetFlagType offset)
{
    int rid = get_rid();
    LFInputOrderField order = {};
    strcpy(order.ExchangeID, exchange_id.c_str());
    strcpy(order.InstrumentID, instrument_id.c_str());
    order.LimitPrice = price;
    order.Volume = volume;
    order.MinVolume = 1;
    order.TimeCondition = LF_CHAR_IOC;
    order.VolumeCondition = LF_CHAR_AV;
    order.OrderPriceType = LF_CHAR_LimitPrice;
    order.Direction = direction;
    order.OffsetFlag = offset;
    order.HedgeFlag = LF_CHAR_Speculation;
    order.ForceCloseReason = LF_CHAR_NotForceClose;
    order.StopPrice = 0;
    order.IsAutoSuspend = true;
    order.ContingentCondition = LF_CHAR_Immediately;
    strcpy(order.MiscInfo, strategy_name.c_str());
    write_frame_extra(&order, sizeof(LFInputOrderField), source, MSG_TYPE_LF_ORDER, 1/*lastflag*/, rid, md_nano);
    return rid;
}

int WCStrategyUtil::req_position(short source)
{
    int rid = get_rid();
    LFQryPositionField req = {};
    write_frame(&req, sizeof(LFQryPositionField), source, MSG_TYPE_LF_QRY_POS, 1/*lastflag*/, rid);
    return rid;
}

int WCStrategyUtil::cancel_order(short source, int order_id)
{
    if (order_id < rid_start || order_id > rid_end)
        return -1;
    int rid = get_rid();
    LFOrderActionField req = {};
    req.KfOrderID = order_id;
    req.ActionFlag = LF_CHAR_Delete;
    req.LimitPrice = 0;
    req.VolumeChange = 0;
    req.RequestID = rid;
    write_frame(&req, sizeof(LFOrderActionField), source, MSG_TYPE_LF_ORDER_ACTION, 1/*lastflag*/, rid);
    return rid;
}

void WCStrategyUtil::set_pos_back(short source, const char* pos_str)
{
    write_frame(pos_str, strlen(pos_str) + 1, source, MSG_TYPE_STRATEGY_POS_SET, 1, -1);
}