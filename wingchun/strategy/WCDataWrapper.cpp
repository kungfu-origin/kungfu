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

#include "WCDataWrapper.h"
#include "WCStrategyUtil.h"
#include "Timer.h"
#include "longfist/LFUtils.h"
#include "longfist/sys_messages.h"
#include <sstream>

USING_WC_NAMESPACE

volatile int IWCDataProcessor::signal_received = -1;

WCDataWrapper::WCDataWrapper(IWCDataProcessor *processor, WCStrategyUtil* util):
    processor(processor), util(util), force_stop(false), auto_sub_mode_enabled(true)
{
    auto rids = util->getRequestIds();
    rid_start = rids.first;
    rid_end = rids.second;
    cur_time = yijinjing::getNanoTime();
    processor->on_time(cur_time);
}

#define ADD_JOURNAL(func, source) \
    JournalPair jp = func(source); \
    folders.push_back(jp.first); \
    names.push_back(jp.second); \

void WCDataWrapper::add_market_data(short source)
{
    ADD_JOURNAL(getMdJournalPair, source);
}

void WCDataWrapper::add_register_td(short source)
{
    tds[source] = CONNECT_TD_STATUS_ADDED;
    ADD_JOURNAL(getTdJournalPair, source);
}

void WCDataWrapper::register_bar_md(short source, int min_interval, string start_time, string end_time)
{
    struct tm start_tm, end_tm, tmp_tm, cur_tm;
    // setup cur_tm
    time_t now = time(0);
    cur_tm = *localtime(&now);
    cur_tm.tm_sec = 0;
    // init start_tm / end_tm
    start_tm = cur_tm;
    end_tm = cur_tm;
    // parse string; set hour/min; produce nano
    string format = "%H:%M:%S";
    strptime(start_time.c_str(), format.c_str(), &tmp_tm);
    start_tm.tm_hour = tmp_tm.tm_hour;
    start_tm.tm_min = tmp_tm.tm_min;
    long start_nano = kungfu::yijinjing::parseTm(start_tm);
    strptime(end_time.c_str(), format.c_str(), &tmp_tm);
    end_tm.tm_hour = tmp_tm.tm_hour;
    end_tm.tm_min = tmp_tm.tm_min;
    long end_nano = kungfu::yijinjing::parseTm(end_tm);
    if (end_nano < start_nano)
        end_nano += kungfu::yijinjing::NANOSECONDS_PER_DAY;
    long delta = min_interval * kungfu::yijinjing::NANOSECONDS_PER_MINUTE;

    bool inited = false;
    /* later on, process bar for each interval */
    for (long nano = start_nano; nano <= end_nano; nano += delta)
    {
        if (nano > cur_time)
        {
            if (!inited)
            {
                /* init bar at start time */
                BLCallback callback = boost::bind(&WCDataWrapper::init_bar, this, source, min_interval);
                util->insert_callback(nano, callback);
                inited = true;
            }
            else
            {
                BLCallback callback = boost::bind(&WCDataWrapper::send_bar, this, source, min_interval);
                util->insert_callback(nano, callback);
            }
        }
    }
}

void WCDataWrapper::init_bar(short source, int min_interval)
{
    bars[source][min_interval].clear();
}

void WCDataWrapper::send_bar(short source, int min_interval)
{
    BarMdMap& bar = bars[source][min_interval];
    processor->on_market_bar(bar, min_interval, source, kungfu::yijinjing::getNanoTime());
    bar.clear();
}

void WCDataWrapper::process_bar(const LFMarketDataField* data, short source, long rcv_time)
{
    BarIntervalMaps& intervalMaps = bars[source];
    for (auto &interval: intervalMaps)
    {
        BarMdMap& barMap = interval.second;
        string ticker = data->InstrumentID;
        auto iter = barMap.find(ticker);
        if (iter == barMap.end())
        {   // this is the first bar data
            LFBarMarketDataField& bar = barMap[ticker];
            strncpy(bar.TradingDay, data->TradingDay, 9);
            strncpy(bar.InstrumentID, data->InstrumentID, 31);
            bar.UpperLimitPrice = data->UpperLimitPrice;
            bar.LowerLimitPrice = data->LowerLimitPrice;
            strncpy(bar.StartUpdateTime, data->UpdateTime, 13);
            bar.StartUpdateMillisec = data->UpdateMillisec;
            strncpy(bar.EndUpdateTime, data->UpdateTime, 13);
            bar.EndUpdateMillisec = data->UpdateMillisec;
            bar.Open = data->LastPrice;
            bar.Close = data->LastPrice;
            bar.Low = data->LastPrice;
            bar.High = data->LastPrice;
            bar.Volume = 0;
            bar.StartVolume = data->Volume;
        }
        else
        {
            LFBarMarketDataField& bar = barMap[ticker];
            strncpy(bar.EndUpdateTime, data->UpdateTime, 13);
            bar.EndUpdateMillisec = data->UpdateMillisec;
            bar.Close = data->LastPrice;
            bar.Low = (bar.Low > data->LastPrice) ? data->LastPrice : bar.Low;
            bar.High = (bar.High < data->LastPrice) ? data->LastPrice : bar.High;
            bar.Volume = data->Volume - bar.StartVolume;
        }
    }
}

void WCDataWrapper::pre_run()
{
    /** build up reader now */
    reader = kungfu::yijinjing::JournalReader::createReaderWithSys(folders, names, kungfu::yijinjing::getNanoTime(), processor->get_name());
    /** request td login */
    for (auto& iter: tds)
    {
        short td_source = iter.first;
        util->td_connect(td_source);
        iter.second = CONNECT_TD_STATUS_REQUESTED;
    }
}

void WCDataWrapper::stop()
{
    force_stop = true;
}

void WCDataWrapper::run()
{
    pre_run();
    yijinjing::FramePtr frame;
    force_stop = false;
    processor->signal_received = -1;
    while (!force_stop && processor->signal_received <= 0)
    {
        frame = reader->getNextFrame();
        if (frame.get() != nullptr)
        {
            cur_time = frame->getNano();
            /** start to process messages */
            short msg_type = frame->getMsgType();
            short msg_source = frame->getSource();
            int request_id = frame->getRequestId();

            if (msg_type < 100)
            {   // system msgs
                if (msg_type == MSG_TYPE_SWITCH_TRADING_DAY)
                {
                    for (auto iter: internal_pos)
                    {
                        if (iter.second.get() != nullptr)
                            iter.second->switch_day();
                    }
                    processor->on_switch_day(cur_time);
                }
                else if (msg_type == MSG_TYPE_TRADE_ENGINE_ACK)
                {
                    if (tds[msg_source] == CONNECT_TD_STATUS_REQUESTED)
                    {
                        string content((char*)frame->getData());
                        process_td_ack(content, msg_source, cur_time);
                    }
                }
            }
            else
            {
                void *data = frame->getData();
                if (request_id == -1)
                {
                    util->set_md_nano(cur_time);
                    switch (msg_type)
                    {
                        case MSG_TYPE_LF_MD:
                        {
                            LFMarketDataField * md = (LFMarketDataField *) data;
                            last_prices[md->InstrumentID] = md->LastPrice;
                            process_bar(md, msg_source, cur_time);
                            processor->on_market_data(md, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_L2_MD:
                        {
                            processor->on_market_data_level2((LFL2MarketDataField *)data, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_L2_INDEX:
                        {
                            processor->on_l2_index((LFL2IndexField *)data, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_L2_ORDER:
                        {
                            processor->on_l2_order((LFL2OrderField *)data, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_L2_TRADE:
                        {
                            processor->on_l2_trade((LFL2TradeField *)data, msg_source, cur_time);
                            break;
                        }
                    }
                    util->set_md_nano(-1);
                }
                if (rid_start <= request_id && request_id < rid_end)
                {
                    switch (msg_type)
                    {
                        case MSG_TYPE_LF_RSP_POS:
                        {
                            process_rsp_position((LFRspPositionField *) data, frame->getLastFlag(), request_id, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_ORDER:
                        {
                            processor->on_rsp_order((LFInputOrderField *) data, request_id, msg_source, cur_time,
                                                    frame->getErrorId(), frame->getErrorMsg());
                            break;
                        }
                        case MSG_TYPE_LF_RTN_ORDER:
                        {
                            LFRtnOrderField * rtn_order = (LFRtnOrderField *) data;
                            order_statuses[request_id] = rtn_order->OrderStatus;
                            processor->on_rtn_order(rtn_order, request_id, msg_source, cur_time);
                            break;
                        }
                        case MSG_TYPE_LF_RTN_TRADE:
                        {
                            internal_pos[msg_source]->update((LFRtnTradeField *) data);
                            processor->on_rtn_trade((LFRtnTradeField *) data, request_id, msg_source, cur_time);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            cur_time = yijinjing::getNanoTime();
        }
        processor->on_time(cur_time);
    }
    if (processor->signal_received > 0)
    {
        std::stringstream ss;
        ss << "[DataWrapper] signal received: " << processor->signal_received;
        processor->debug(ss.str().c_str());
    }

    if (force_stop)
    {
        processor->debug("[DataWrapper] forced to stop!");
    }

}

void WCDataWrapper::process_rsp_position(const LFRspPositionField* pos, bool is_last, int request_id, short source, long rcv_time)
{
    PosHandlerPtr& handler = pos_handlers[request_id];
    if (handler.get() == nullptr)
    {
        handler = PosHandler::create(source);
    }
    if (strlen(pos->InstrumentID) > 0)
    {
        handler->add_pos(pos->InstrumentID, pos->PosiDirection, pos->Position, pos->YdPosition, pos->PositionCost);
    }
    if (is_last)
    {
        processor->on_rsp_position(handler, request_id, source, rcv_time);
        handler.reset();
    }
}

void WCDataWrapper::process_td_ack(const string& content, short source, long rcv_time)
{
    try
    {
        json j_ack = json::parse(content);
        if (j_ack["name"].get<string>().compare(processor->get_name()) == 0)
        {
            bool ready = j_ack["ok"].get<bool>();
            processor->on_td_login(ready, j_ack, source);
            if (j_ack.find(PH_FEE_SETUP_KEY) != j_ack.end())
            {
                fee_handlers[source] = FeeHandlerPtr(new FeeHandler(j_ack[PH_FEE_SETUP_KEY]));
            }
            if (ready)
            {
                PosHandlerPtr handler = PosHandler::create(source, content);
                set_internal_pos(handler, source);
                tds[source] = CONNECT_TD_STATUS_ACK_POS;
                processor->on_rsp_position(handler, -1, source, cur_time);
            }
            else
            {
                tds[source] = CONNECT_TD_STATUS_ACK_NONE;
                processor->on_rsp_position(PosHandlerPtr(), -1, source, cur_time);
            }
        }
    }
    catch (...)
    {
        std::stringstream ss;
        ss << "[ERROR] processing td ack: " << content;
        processor->debug(ss.str().c_str());
    }
}

void WCDataWrapper::set_pos(PosHandlerPtr handler, short source)
{
    set_internal_pos(handler, source);
    if (fee_handlers[source].get() != nullptr)
    {
        handler->set_fee(fee_handlers[source]);
    }
    if (handler.get() != nullptr && !handler->poisoned())
    {
        json j_req = json::parse(handler->to_string());
        j_req["name"] = processor->get_name();
        util->set_pos_back(source, j_req.dump().c_str());
        tds[source] = CONNECT_TD_STATUS_SET_BACK;
    }
}

void WCDataWrapper::set_internal_pos(PosHandlerPtr handler, short source)
{
    internal_pos[source] = handler;
    // here we need to subscribe all market data (both for trade engine and strategy)
    if (handler.get() != nullptr)
    {
        vector<string> tickers = handler->get_tickers();
        if (auto_sub_mode_enabled)
        {
            util->subscribeMarketData(tickers, source);
        }
        else
        {
            processor->debug("[auto-sub] disabled!");
        }
    }
}

PosHandlerPtr WCDataWrapper::get_pos(short source)
{
    PosHandlerPtr handler = internal_pos[source];
    if (handler.get() == nullptr)
        handler = PosHandler::create(source);
    return handler;
}

byte WCDataWrapper::get_td_status(short source) const
{
    auto iter = tds.find(source);
    if (iter == tds.end())
        return CONNECT_TD_STATUS_UNKNOWN;
    else
        return iter->second;
}

double WCDataWrapper::get_ticker_pnl(short source, string ticker, bool include_fee) const
{
    using kungfu::yijinjing::VOLUME_DATA_TYPE;
    auto pos_iter = internal_pos.find(source);
    auto price_iter = last_prices.find(ticker);
    if (pos_iter != internal_pos.end() && price_iter != last_prices.end())
    {
        PosHandlerPtr handler = pos_iter->second;
        double last_price = price_iter->second;

        double holding_value = handler->get_holding_value(ticker, last_price);
        double holding_balance = handler->get_holding_balance(ticker);

        double pnl = holding_value - holding_balance;
        if (include_fee)
            pnl -= handler->get_holding_fee(ticker);

        return pnl;
    }
    return 0;
}
/** get effective orders */
vector<int> WCDataWrapper::get_effective_orders() const
{
    vector<int> res;
    for (auto iter: order_statuses)
    {
        char status = iter.second;
        if (status != LF_CHAR_Error
            && status != LF_CHAR_AllTraded
            && status != LF_CHAR_Canceled
            && status != LF_CHAR_PartTradedNotQueueing)
        {
            res.push_back(iter.first);
        }
    }
    return res;
}