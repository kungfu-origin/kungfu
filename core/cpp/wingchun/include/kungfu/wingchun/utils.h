//
// Created by qlu on 2019-09-04.
//

#ifndef KUNGFU_WINGCHUN_UTILS_H
#define KUNGFU_WINGCHUN_UTILS_H

#include <hffix.hpp>
#include <spdlog/spdlog.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/common.h>

using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        inline void write_subscribe_msg(const yijinjing::journal::writer_ptr &writer, int64_t trigger_time,
                const std::string &exchange, const std::string &symbol)
        {
            const size_t buffer_size = 1024;
            char *buffer = const_cast<char *>(&(writer->open_frame(trigger_time, msg::type::Subscribe, buffer_size)->data<char>()));
            hffix::message_writer sub_msg(buffer, buffer + buffer_size);
            sub_msg.push_back_header("FIX.4.2");
            sub_msg.push_back_string(hffix::tag::MsgType, "V");
            sub_msg.push_back_int(hffix::tag::MDReqID, 1);
            sub_msg.push_back_int(hffix::tag::SubscriptionRequestType, 1); // Snapshot + Updates (Subscribe)
            sub_msg.push_back_int(hffix::tag::MarketDepth, 1);

            sub_msg.push_back_int(hffix::tag::NoMDEntryTypes, 1);
            sub_msg.push_back_int(hffix::tag::MDEntryType, 2);
            sub_msg.push_back_int(hffix::tag::NoRelatedSym, 1);
            sub_msg.push_back_string(hffix::tag::SecurityExchange, exchange);
            sub_msg.push_back_string(hffix::tag::Symbol, symbol);
            sub_msg.push_back_trailer();
            writer->close_frame(sub_msg.message_end() - buffer);
            SPDLOG_TRACE("written subscribe msg for {}@{}", symbol, exchange);
        }
    }
}

#endif //KUNGFU_WINGCHUN_UTILS_H
