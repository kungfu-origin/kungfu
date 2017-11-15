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

#include "IMDEngine.h"
#include "Timer.h"
#include "longfist/LFUtils.h"
#include "longfist/sys_messages.h" /**< for msg_type usage in listening */

USING_WC_NAMESPACE

IMDEngine::IMDEngine(short source): IEngine(source)
{
    subs_tickers.clear();
    subs_markets.clear();
    history_subs.clear();
}

void IMDEngine::set_reader_thread()
{
    reader_thread = ThreadPtr(new std::thread(boost::bind(&IMDEngine::listening, this)));
}

void IMDEngine::init()
{
    reader = yijinjing::JournalReader::createSysReader(name());
    JournalPair l1MdPair = getMdJournalPair(source_id);
    writer = yijinjing::JournalWriter::create(l1MdPair.first, l1MdPair.second, name());
}

void IMDEngine::listening()
{
    yijinjing::FramePtr frame;
    while (isRunning && signal_received < 0)
    {
        frame = reader->getNextFrame();
        if (frame.get() != nullptr)
        {
            short msg_type = frame->getMsgType();
            short msg_source = frame->getSource();
            if (msg_source == source_id)
            {
                switch (msg_type)
                {
                    case MSG_TYPE_SUBSCRIBE_MARKET_DATA:
                    case MSG_TYPE_SUBSCRIBE_L2_MD:
                    case MSG_TYPE_SUBSCRIBE_INDEX:
                    case MSG_TYPE_SUBSCRIBE_ORDER_TRADE:
                    {
                        string ticker((char*)(frame->getData()));
                        size_t found = ticker.find(TICKER_MARKET_DELIMITER);
                        if (found != string::npos)
                        {
                            subs_tickers.push_back(ticker.substr(0, found));
                            subs_markets.push_back(ticker.substr(found + 1));
                            KF_LOG_DEBUG(logger, "[sub] (ticker)" << ticker.substr(0, found) << " (market)" << ticker.substr(found + 1));
                        }
                        else
                        {
                            subs_tickers.push_back(ticker);
                            subs_markets.push_back("");
                            KF_LOG_DEBUG(logger, "[sub] (ticker)" << ticker << " (market)null");
                        }

                        // maintain sub_counts
                        SubCountMap& sub_counts = history_subs[msg_type];
                        if (sub_counts.find(ticker) == sub_counts.end())
                        {
                            sub_counts[ticker] = 1;
                        }
                        else
                        {
                            sub_counts[ticker] += 1;
                        }

                        if (frame->getLastFlag() == 1)
                        {
                            if (is_logged_in())
                            {
                                if (msg_type == MSG_TYPE_SUBSCRIBE_MARKET_DATA)
                                {
                                    subscribeMarketData(subs_tickers, subs_markets);
                                }
                                else if (msg_type == MSG_TYPE_SUBSCRIBE_L2_MD)
                                {
                                    subscribeL2MD(subs_tickers, subs_markets);
                                }
                                else if (msg_type == MSG_TYPE_SUBSCRIBE_INDEX)
                                {
                                    subscribeIndex(subs_tickers, subs_markets);
                                }
                                else if (msg_type == MSG_TYPE_SUBSCRIBE_ORDER_TRADE)
                                {
                                    subscribeOrderTrade(subs_tickers, subs_markets);
                                }
                                subs_tickers.clear();
                                subs_markets.clear();
                            }
                        }
                    }
                }
            }

            if (msg_type == MSG_TYPE_MD_ENGINE_OPEN && (msg_source <= 0 || msg_source == source_id))
            {
                on_engine_open();
            }
            else if (msg_type == MSG_TYPE_MD_ENGINE_CLOSE && (msg_source <= 0 || msg_source == source_id))
            {
                on_engine_close();
            }
            else if (msg_type == MSG_TYPE_STRING_COMMAND)
            {
                string cmd((char*)frame->getData());
                on_command(cmd);
            }
        }
    }

    if (IEngine::signal_received >= 0)
    {
        KF_LOG_INFO(logger, "[IEngine] signal received: " << IEngine::signal_received);
    }

    if (!isRunning)
    {
        KF_LOG_INFO(logger, "[IEngine] forced to stop.");
    }
}

void IMDEngine::pre_run()
{
    subscribeHistorySubs();
}

void IMDEngine::subscribeHistorySubs()
{
    for (auto& iter: history_subs)
    {
        if (iter.second.size() == 0)
            continue;

        short msg_type = iter.first;

        vector<string> tickers;
        vector<string> markets;

        for (auto& tickerIter: iter.second)
        {
            const string& ticker = tickerIter.first;
            size_t found = ticker.find(TICKER_MARKET_DELIMITER);
            if (found != string::npos)
            {
                tickers.push_back(ticker.substr(0, found));
                markets.push_back(ticker.substr(found + 1));
            }
            else
            {
                tickers.push_back(ticker);
                markets.push_back("");
            }
        }
        if (msg_type == MSG_TYPE_SUBSCRIBE_MARKET_DATA)
        {
            subscribeMarketData(tickers, markets);
        }
        else if (msg_type == MSG_TYPE_SUBSCRIBE_L2_MD)
        {
            subscribeL2MD(tickers, markets);
        }
        else if (msg_type == MSG_TYPE_SUBSCRIBE_INDEX)
        {
            subscribeIndex(tickers, markets);
        }
        else if (msg_type == MSG_TYPE_SUBSCRIBE_ORDER_TRADE)
        {
            subscribeOrderTrade(tickers, markets);
        }
        KF_LOG_INFO(logger, "[sub] history (msg_type)" << msg_type << " (num)" << tickers.size());
    }
}

void IMDEngine::on_market_data(const LFMarketDataField* data)
{
    if (isRunning)
    {
        writer->write_frame(data, sizeof(LFMarketDataField), source_id, MSG_TYPE_LF_MD, 1/*islast*/, -1/*invalidRid*/);
        KF_LOG_DEBUG_FMT(logger, "%-10s[%7.1f, %4d | %7.1f, %4d]",
                         data->InstrumentID,
                         data->BidPrice1,
                         data->BidVolume1,
                         data->AskPrice1,
                         data->AskVolume1);
    }
}