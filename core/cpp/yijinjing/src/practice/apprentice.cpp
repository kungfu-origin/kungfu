/*****************************************************************************
 * Copyright [taurus.ai]
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

//
// Created by Keren Dong on 2019-06-01.
//

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/apprentice.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu
{
    namespace practice
    {
        apprentice::apprentice(location_ptr home, bool low_latency) : hero(std::make_shared<io_device_client>(home, low_latency))
        {
            auto now = time::now_in_nano();
            auto uid_str = fmt::format("{:08x}", get_home_uid());
            master_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "app", uid_str, home->locator);
            reader_->subscribe(master_location_, get_home_uid(), now);
            nlohmann::json request;
            request["msg_type"] = MsgType::Register;
#ifdef _WINDOWS
            request["pid"] = _getpid();
#else
            request["pid"] = getpid();
#endif
            request["gen_time"] = now;
            request["trigger_time"] = now;
            request["source"] = get_home_uid();
            request["mode"] = home->mode;
            request["category"] = home->category;
            request["group"] = home->group;
            request["name"] = home->name;
            get_io_device()->get_publisher()->publish(request.dump());
        }

        void apprentice::subscribe(const location_ptr location)
        {
            auto now = time::now_in_nano();
            action::RequestSubscribe request;
            request.source_id = location->uid;
            request.from_time = now;
            get_writer(master_location_->uid)->write(now, MsgType::RequestSubscribe, get_home_uid(), &request);
        }
    }
}

int main(int argc, char **argv)
{
    int64_t n = time::now_in_nano();
    std::string s = time::strftime(n);
    int64_t p = time::strptime(s);
    std::string c = time::strftime(n);
    std::cout << n << std::endl;
    std::cout << s << std::endl;
    std::cout << p << std::endl;
    std::cout << c << std::endl;
    std::cout << time::strfnow() << std::endl;
//    log::setup_log("test");
//    SPDLOG_WARN("test");
//    std::string name = "test";
//    apprentice app(name);
//
//    writer_ptr w = app.get_io_device().open_writer(data::mode::LIVE, data::category::MD, "xtp", "xtp");
//    char *buffer = reinterpret_cast<char *>(w->open_frame(0, 0, 0).address());
//    int seq_send(1); // Sending sequence number
//    hffix::message_writer new_order(buffer, buffer + 1024);
//
//    new_order.push_back_header("FIX.4.2");
//
//    // New Order - Single
//    new_order.push_back_string(hffix::tag::MsgType, "D");
//    // Required Standard Header field.
//    new_order.push_back_string(hffix::tag::SenderCompID, "AAAA");
//    new_order.push_back_string(hffix::tag::TargetCompID, "BBBB");
//    new_order.push_back_int(hffix::tag::MsgSeqNum, seq_send++);
////    new_order.push_back_timestamp (hffix::tag::SendingTime, tsend);
//    new_order.push_back_string(hffix::tag::ClOrdID, "A1");
//    // Automated execution.
//    new_order.push_back_char(hffix::tag::HandlInst, '1');
//    // Ticker symbol OIH.
//    new_order.push_back_string(hffix::tag::Symbol, "OIH");
//    // Buy side.
//    new_order.push_back_char(hffix::tag::Side, '1');
////    new_order.push_back_timestamp (hffix::tag::TransactTime, tsend);
//    // 100 shares.
//    new_order.push_back_int(hffix::tag::OrderQty, 100);
//    // Limit order.
//    new_order.push_back_char(hffix::tag::OrdType, '2');
//    // Limit price $500.01 = 50001*(10^-2). The push_back_decimal() method
//    // takes a decimal floating point number of the form mantissa*(10^exponent).
//    new_order.push_back_decimal(hffix::tag::Price, 50001, -2);
//    // Good Till Cancel.
//    new_order.push_back_char(hffix::tag::TimeInForce, '1');
//
//    new_order.push_back_trailer(); // write CheckSum.
//
//    w->close_frame(new_order.message_end() - buffer);
//
//    std::cout << "journal written" << std::endl;
////    app.subscribe_journal(type::MD, "xtp", "xtp", kungfu::yijinjing::TIME_FROM_FIRST);
////    app.add_frame_handler(&go_and_die);
//    std::cout << "apprentice start" << std::endl;
//    app.go();
//    std::cout << "done" << std::endl;
}