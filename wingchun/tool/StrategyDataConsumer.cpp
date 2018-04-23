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

#include "StrategyDataConsumer.h"
#include "Timer.h"
#include "longfist/LFUtils.h"
#include <iostream>

USING_WC_NAMESPACE
using namespace std;
using namespace kungfu::yijinjing;

#define PARSE_NANO(nano) parseNano(nano, "%Y%m%d-%H:%M:%S")

StrategyBasicDataConsumer::StrategyBasicDataConsumer(const string& strategy_name, int target_session_id)
        : DataConsumer(), cur_session(0), tar_session(target_session_id), strategy_name(strategy_name)
{
}

void StrategyBasicDataConsumer::on_strategy_start(json& j_start, long rcv_time, const string& name)
{
    if (name.compare(strategy_name) == 0) // equal
    {
        cur_session ++;
        unit.name = name;
        unit.folder = j_start["folder"].get<string>();
        unit.last_switch_nano = j_start["last_switch_nano"].get<long>();
        unit.rid_start = j_start["rid_s"].get<int>();
        unit.rid_end = j_start["rid_e"].get<int>();
        if (tar_session <= 0 || tar_session == cur_session)
        {
            cout << cur_session << ":" << endl;
            cout << "\tStart:" << " (name)" << name
                 << " (rid)" << unit.rid_start << "-" << unit.rid_end
                 << " (nano)" << rcv_time << " (time)" << PARSE_NANO(rcv_time)
                 << endl;
        }
        int idx = reader->addJournal(unit.folder, unit.name);
        reader->seekTimeJournal(idx, rcv_time);
    }
}

void StrategyBasicDataConsumer::on_strategy_end(json& j_end, long rcv_time, const string& name)
{
    if (name.compare(strategy_name) == 0 && (tar_session <= 0 || tar_session == cur_session))
    {
        if (unit.pos_handler.get() != nullptr)
        {
            cout << "\t\t[end_pos]\t(valid)" << !unit.pos_handler->poisoned()
                 << " (content)" << unit.pos_handler->to_string()
                 << " (time)" << PARSE_NANO(rcv_time) << endl;
        }
        cout << "\tEnd:  " << " (name)" << name
             << " (rid)" << unit.rid_start << "-" << unit.rid_end
             << " (nano)" << rcv_time << " (time)" << PARSE_NANO(rcv_time) << endl;
    }
}

StrategyDataConsumer::StrategyDataConsumer(const string& strategy_name,
                                           int target_session_id)
        : StrategyBasicDataConsumer(strategy_name, target_session_id)
{
}

void StrategyDataConsumer::on_pos_set(json& j_pos, short source, long rcv_time, const string& name)
{
    if (name.compare(strategy_name) == 0 && (tar_session <= 0 || tar_session == cur_session))
    {
        unit.pos_handler = PosHandler::create(source, j_pos.dump());
        cout << "\t\t[pos_set]\t(valid)" << !unit.pos_handler->poisoned()
             << " (content)" << unit.pos_handler->to_string()
             << " (time)" << PARSE_NANO(rcv_time) << endl;
    }
}

void StrategyDataConsumer::on_td_connect(json& j_request, short source, long rcv_time, const string& name)
{
    if (name.compare(strategy_name) == 0 && (tar_session <= 0 || tar_session == cur_session))
    {
        // add td journal
        JournalPair jp = getTdJournalPair(source);
        int idx = reader->addJournal(jp.first, jp.second);
        reader->seekTimeJournal(idx, rcv_time);
        // add td send journal
        JournalPair send_jp = getTdSendJournalPair(source);
        idx = reader->addJournal(send_jp.first, send_jp.second);
        reader->seekTimeJournal(idx, rcv_time);
        // output
        cout << "\t\t[td_connect]\t" << name << " -> " << jp.second << " (time)" << PARSE_NANO(rcv_time) << endl;
    }
}

void StrategyDataConsumer::on_td_ack(json& j_ack, short source, long rcv_time, const string& name)
{
    if (name.compare(strategy_name) == 0 && (tar_session <= 0 || tar_session == cur_session))
    {
        if (!j_ack["ok"].get<bool>())
        {
            cout << "\t\t[td_ack]\tno available position" << endl;
        }
        else
        {
            unit.pos_handler = PosHandler::create(source, j_ack.dump());
            cout << "\t\t[td_ack]\t(valid)" << !unit.pos_handler->poisoned() << " (content)" << unit.pos_handler->to_string() << endl;
        }
    }
}

void StrategyDataConsumer::on_order_origin(LFInputOrderField* order, short source, int order_id, long rcv_time, long md_nano, const string& name)
{
    if (tar_session <= 0 || tar_session == cur_session)
    {
        if (name.compare(strategy_name) == 0)
        {
            OrderLatencyUnit& order_unit = order_map[order_id];
            order_unit.order_id = order_id;
            order_unit.ticker = order->InstrumentID;
            order_unit.price = order->LimitPrice;
            order_unit.volume = order->Volume;
            order_unit.direction = order->Direction;
            order_unit.offset = order->OffsetFlag;
            order_unit.order_status = '-';
            order_unit.oe_time = rcv_time;
            order_unit.md_time = md_nano;
        }
        else
        {
            if (order_map.find(order_id) != order_map.end())
            {
                order_map[order_id].rsp_time = rcv_time;
            }
        }
    }
}

void StrategyDataConsumer::on_order_send(LFInputOrderField* order, short source, int order_id, long after_nano, long before_nano)
{
    if (tar_session <= 0 || tar_session == cur_session)
    {
        if (order_map.find(order_id) != order_map.end())
        {
            OrderLatencyUnit& order_unit = order_map[order_id];
            order_unit.send_after_time = after_nano;
            order_unit.send_before_time = before_nano;
            order_unit.order_status = LF_CHAR_OrderInserted;
        }
    }
}

void StrategyDataConsumer::on_rtn_order(LFRtnOrderField* order, short source, int order_id, long rcv_time)
{
    if (tar_session <= 0 || tar_session == cur_session)
    {
        if (order_map.find(order_id) != order_map.end())
        {
            OrderLatencyUnit &order_unit = order_map[order_id];
            order_unit.order_status = order->OrderStatus;
            if (order_unit.number_rtn_order == 0)
                order_unit.first_rtn_order_time = rcv_time;
            order_unit.number_rtn_order ++;
        }
    }
}

void StrategyDataConsumer::on_rtn_trade(LFRtnTradeField* trade, short source, int order_id, long rcv_time)
{
    if (tar_session <= 0 || tar_session == cur_session)
    {
        if (order_map.find(order_id) != order_map.end())
        {
            OrderLatencyUnit &order_unit = order_map[order_id];
            if (order_unit.number_rtn_trade == 0)
                order_unit.first_rtn_trade_time = rcv_time;
            order_unit.number_rtn_trade ++;
            rtn_trade_map[order_id][rcv_time] = *trade;
            unit.pos_handler->update(trade);
        }
    }
}

void StrategyDataConsumer::print_order() const
{
    cout << endl;
    cout << "+-----------------------------------------------------------------------------------------------------------------+" << endl;
    cout << "|                                  Kungfu order latency report                                                    |" << endl;
    cout << "+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+" << endl;
    cout << "| order_id  | rcv_time        | TTT-b(ns)    | TTT-a(ns)    | ticker   | price    | volume   | offset       | dir |" << endl;
    cout << "+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+" << endl;
    for (auto& iter: order_map)
    {
        const OrderLatencyUnit &unit = iter.second;
        printf("|%11d|%s|%14ld|%14ld|%10s|%10.1f|%10d|%14s|%5s|\n",
               unit.order_id,
               PARSE_NANO(unit.oe_time).c_str(),
               (unit.md_time > 0 && unit.send_before_time > 0) ? (unit.send_before_time - unit.md_time) : -1,
               (unit.md_time > 0 && unit.send_after_time > 0) ? (unit.send_after_time - unit.md_time) : -1,
               unit.ticker.c_str(),
               unit.price,
               unit.volume,
               getLfOffsetFlagType(unit.offset).c_str(),
               getLfDirectionType(unit.direction).c_str()
        );
        cout << "+-----------+-----------------+--------------+--------------+----------+----------+----------+--------------+-----+" << endl;
    }
}

void StrategyDataConsumer::print_trade() const
{
    cout << endl;
    cout << "+-------------------------------------------------------------------------------------------+" << endl;
    cout << "|                                  Kungfu trade latency report                              |" << endl;
    cout << "+-----------+---+-----------------+--------------+----------+---------+----------+----------+" << endl;
    cout << "| order_id  |idx| rcv_time        | OTR(ns)      | ticker   | t_time  | t_price  | t_volume |" << endl;
    cout << "+-----------+---+-----------------+--------------+----------+---------+----------+----------+" << endl;
    for (auto& iter: order_map)
    {
        int order_id = iter.first;
        const OrderLatencyUnit &unit = iter.second;
        auto trade_map_iter = rtn_trade_map.find(order_id);
        if (trade_map_iter != rtn_trade_map.end())
        {
            int idx = 1;
            for (auto& rtn_trade_iter: trade_map_iter->second)
            {
                long rcv_time = rtn_trade_iter.first;
                const LFRtnTradeField& rtn_trade = rtn_trade_iter.second;
                printf("|%11d|%3d|%s|%14ld|%10s|%9s|%10.1f|%10d|\n",
                       order_id,
                       idx,
                       PARSE_NANO(rcv_time).c_str(),
                       (unit.send_after_time > 0) ? (rcv_time - unit.send_after_time) : -1,
                       rtn_trade.InstrumentID,
                       rtn_trade.TradeTime,
                       rtn_trade.Price,
                       rtn_trade.Volume
                );
                cout << "+-----------+---+-----------------+--------------+----------+---------+----------+----------+" << endl;
                idx ++;
            }
        }
    }
}

class Calculator
{
private:
    int tot_n;
    int n;
    long min;
    long max;
    long sum;
    long square_sum;
public:
    Calculator(): tot_n(0), n(0), min(-1), max(0), sum(0), square_sum(0) {}
    void update(long d)
    {
        n++;
        sum += d;
        square_sum += (long)d*d;
        min = (min < 0) ? d : ((d < min) ? d : min);
        max = (d > max) ? d : max;
    };
    void update(long a1, long a2)
    {
        if (a1 > 0 && a2 > 0)
            update(a1 - a2);
        tot_n ++;
    }
    void print() const
    {
        printf("%14ld|%14.5e|%14ld|%14ld|%7d|%7d|\n",
               (n == 0)? 0: sum / n,
               (n == 0)? 0: sqrt((square_sum - (double)sum * sum / n) / n),
               (min < 0) ? 0: min,
               max,
               n,
               tot_n
        );
        cout << "+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+" << endl;
    }
};

void StrategyDataConsumer::print_stat() const
{
    cout << endl;
    cout << "+------------------------------------------------------------------------------------------------------+" << endl;
    cout << "|                                       Kungfu latency statistics                                      |" << endl;
    cout << "+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+" << endl;
    cout << "| type | description       | mean(ns)     | std(ns)      | min(ns)      | max(ns)      | valid | total |" << endl;
    cout << "+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+" << endl;
    Calculator ttt_b, ttt_a, str, otr_rsp, otr_f_order, otr_f_trade, otr_trade;
    for (auto& iter: order_map)
    {
        int order_id = iter.first;
        const OrderLatencyUnit& unit = iter.second;
        ttt_b.update(unit.send_before_time, unit.md_time);
        ttt_a.update(unit.send_after_time, unit.md_time);
        str.update(unit.oe_time, unit.md_time);
        otr_rsp.update(unit.rsp_time, unit.send_after_time);
        otr_f_order.update(unit.first_rtn_order_time, unit.send_after_time);
        otr_f_trade.update(unit.first_rtn_trade_time, unit.send_after_time);
        auto trade_map_iter = rtn_trade_map.find(order_id);
        if (trade_map_iter != rtn_trade_map.end()) {
            for (auto &rtn_trade_iter: trade_map_iter->second) {
                otr_trade.update(rtn_trade_iter.first, unit.send_after_time);
            }
        }
    }
    cout << "| TTT  | (tick-to-trade)   |                                                                           |" << endl;
    cout << "+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+" << endl;
    cout << "|      | before API        |";
    ttt_b.print();
    cout << "|      | after API         |";
    ttt_a.print();
    cout << "| STR  | strategy calc     |";
    str.print();
    cout << "| OTR  | (order-to-return) |                                                                           |" << endl;
    cout << "+------+-------------------+--------------+--------------+--------------+--------------+-------+-------+" << endl;
    cout << "|      | rsp order insert  |";
    otr_rsp.print();
    cout << "|      | first rtn order   |";
    otr_f_order.print();
    cout << "|      | first rtn trade   |";
    otr_f_trade.print();
    cout << "|      | all rtn trade     |";
    otr_trade.print();
}

void StrategyDataConsumer::print_detail() const
{
    cout << endl;
    int idx = 1;
    for (auto& iter: order_map)
    {
        int order_id = iter.first;
        const OrderLatencyUnit& unit = iter.second;
        cout << idx << "."
             << " (oid)" << order_id
             << " (#rtn_o)" << unit.number_rtn_order
             << " (#rtn_t)" << unit.number_rtn_trade
             << " (md)" << unit.md_time
             << " (oe)" << unit.oe_time
             << " (send_b)" << unit.send_before_time
             << " (send_a)" << unit.send_after_time
             << " (rsp)" << unit.rsp_time
             << " (f_rtn_o)" << unit.first_rtn_order_time
             << " (f_rtn_t)" << unit.first_rtn_trade_time
             << " (status)" << unit.order_status << endl;
        idx++;
    }
}