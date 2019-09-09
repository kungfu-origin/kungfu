//
// Created by qlu on 2019-09-04.
//

#ifndef KUNGFU_WINGCHUN_UTILS_H
#define KUNGFU_WINGCHUN_UTILS_H

#include <hffix.hpp>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/common.h>

using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        inline size_t fill_subscribe_msg(char* buffer, size_t buffer_size, const std::vector<Instrument> instruments)
        {
            hffix::message_writer sub_msg(buffer, buffer + buffer_size);
            sub_msg.push_back_header("FIX.4.2");
            sub_msg.push_back_string(hffix::tag::MsgType, "V");
            sub_msg.push_back_int(hffix::tag::MDReqID, 1);
            sub_msg.push_back_int(hffix::tag::SubscriptionRequestType, 1); // Snapshot + Updates (Subscribe)
            sub_msg.push_back_int(hffix::tag::MarketDepth, 1);

            sub_msg.push_back_int(hffix::tag::NoMDEntryTypes, 1);
            sub_msg.push_back_int(hffix::tag::MDEntryType, 2);
            sub_msg.push_back_int(hffix::tag::NoRelatedSym, instruments.size());
            for (const auto &inst : instruments)
            {
                sub_msg.push_back_string(hffix::tag::Symbol, inst.instrument_id);
                sub_msg.push_back_string(hffix::tag::SecurityExchange, inst.exchange_id);
            }
            sub_msg.push_back_trailer();
            return sub_msg.message_end() - buffer;
        }

        inline size_t fill_subscribe_msg(char* buffer, size_t buffer_size, const std::vector<std::string> &symbols, const std::string &exchange_id)
        {
            std::vector<Instrument> instruments = {};
            for (const auto& symbol : symbols)
            {
                Instrument inst = {};
                strcpy(inst.instrument_id, symbol.c_str());
                strcpy(inst.exchange_id, exchange_id.c_str());
                instruments.push_back(inst);
            }
            return fill_subscribe_msg(buffer, buffer_size,instruments);
        }

        template <class T> std::vector<Instrument> get_insts(const std::vector<T>& vec)
        {
            std::vector<Instrument> insts;
            for (const auto& item: vec)
            {
                Instrument inst = {};
                strcpy(inst.instrument_id, item.instrument_id);
                strcpy(inst.exchange_id, item.exchange_id);
                insts.push_back(inst);
            }

            std::sort(insts.begin(), insts.end());
            auto it= std::unique(insts.begin(), insts.end());
            insts.erase(it, insts.end());
            return insts;
        }
    }
}

#endif //KUNGFU_WINGCHUN_UTILS_H
