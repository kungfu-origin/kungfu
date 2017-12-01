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

#include "DataConsumer.h"
#include "Timer.h"
#include "longfist/sys_messages.h"
#include <iostream>
#include <sstream>
#include <math.h>

USING_WC_NAMESPACE
using namespace std;
using namespace kungfu::yijinjing;

DataConsumer::DataConsumer(): start_nano(-1), end_nano(-1), cur_time(-1)
{
}

void DataConsumer::init(long start_nano, long end_nano)
{
    this->start_nano = start_nano;
    this->end_nano = end_nano;
    string reader_name = "Consumer_" + parseNano(getNanoTime(), "%H%M%S");
    vector<string> empty;
    reader = JournalReader::createReaderWithSys(empty, empty, start_nano, reader_name);
    cur_time = start_nano;
}

void DataConsumer::run()
{
    yijinjing::FramePtr frame;

    while (end_nano < 0 || cur_time < end_nano)
    {
        frame = reader->getNextFrame();
        if (frame.get() != nullptr)
        {
            short msg_type = frame->getMsgType();
            short msg_source = frame->getSource();
            int request_id = frame->getRequestId();
            string name = reader->getFrameName();
            void* data = frame->getData();
            cur_time = frame->getNano();
            long extra_nano = frame->getExtraNano();

            if (msg_type == MSG_TYPE_LF_MD) // md
            {
                LFMarketDataField* md = (LFMarketDataField*)data;
                on_market_data(md);
            }
            else if (msg_type == MSG_TYPE_STRATEGY_START) // system
            {
                try
                {
                    string content((char *) frame->getData());
                    json j_start = json::parse(content);
                    on_strategy_start(j_start, cur_time, j_start["name"].get<string>());
                }
                catch (...)
                {
                    cout << " --- ERROR: cannot parse MSG_TYPE_STRATEGY_START --- " << endl;
                    cout << (char *)frame->getData() << endl;
                    cout << " --- " << endl;
                }
            }
            else if (msg_type == MSG_TYPE_STRATEGY_END) // system
            {
                try
                {
                    string content((char *) frame->getData());
                    json j_end = json::parse(content);
                    on_strategy_end(j_end, cur_time, j_end["name"].get<string>());
                }
                catch (...)
                {
                    cout << " --- ERROR: cannot parse MSG_TYPE_STRATEGY_END --- " << endl;
                    cout << (char *)frame->getData() << endl;
                    cout << " --- " << endl;
                }
            }
            else if (msg_type == MSG_TYPE_TRADE_ENGINE_LOGIN)
            {
                try
                {
                    string content((char *) frame->getData());
                    json j_request = json::parse(content);
                    on_td_connect(j_request, msg_source, cur_time, j_request["name"].get<string>());
                }
                catch (...)
                {
                    cout << " --- ERROR: cannot parse MSG_TYPE_TRADE_ENGINE_LOGIN --- " << endl;
                    cout << (char *)frame->getData() << endl;
                    cout << " --- " << endl;
                }
            }
            else if (msg_type == MSG_TYPE_TRADE_ENGINE_ACK) // td
            {
                try
                {
                    string content((char *) frame->getData());
                    json j_ack = json::parse(content);
                    on_td_ack(j_ack, msg_source, cur_time, j_ack["name"].get<string>());
                }
                catch (...)
                {
                    cout << " --- ERROR: cannot parse MSG_TYPE_TRADE_ENGINE_LOGIN --- " << endl;
                    cout << (char *)frame->getData() << endl;
                    cout << " --- " << endl;
                }
            }
            else if (msg_type == MSG_TYPE_STRATEGY_POS_SET) // oe
            {
                try
                {
                    string content((char *) frame->getData());
                    json j_request = json::parse(content);
                    on_pos_set(j_request, msg_source, cur_time, j_request["name"].get<string>());
                }
                catch (...)
                {
                    cout << " --- ERROR: cannot parse MSG_TYPE_STRATEGY_POS_SET --- " << endl;
                    cout << (char *)frame->getData() << endl;
                    cout << " --- " << endl;
                }
            }
            else if (msg_type == MSG_TYPE_LF_ORDER) // oe / td_send
            {
                LFInputOrderField* order = (LFInputOrderField*)data;
                if (!name.compare(0, 8, "TD_SEND_"))
                {
                    on_order_send(order, msg_source, request_id, cur_time, extra_nano);
                }
                else
                {
                    on_order_origin(order, msg_source, request_id, cur_time, extra_nano, name);
                }
            }
            else if (msg_type == MSG_TYPE_LF_RTN_ORDER) // td
            {
                LFRtnOrderField* order = (LFRtnOrderField*)data;
                on_rtn_order(order, msg_source, request_id, cur_time);
            }
            else if (msg_type == MSG_TYPE_LF_RTN_TRADE) // td
            {
                LFRtnTradeField* trade = (LFRtnTradeField*)data;
                on_rtn_trade(trade, msg_source, request_id, cur_time);
            }
        }
        else
        {
            if (end_nano > 0)
                return;
        }
    }
}
