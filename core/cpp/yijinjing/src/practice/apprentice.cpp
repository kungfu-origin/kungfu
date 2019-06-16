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

#include <iostream>

#include <nlohmann/json.hpp>
#include <nanomsg/pubsub.h>
#include <hffix.hpp>

#include <spdlog/spdlog.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/apprentice.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;
using namespace kungfu::practice;

apprentice::apprentice(yijinjing::data::location_ptr home, bool low_latency) : hero(home, low_latency)
{
    log::setup_log(home, home->name);
    io_device_ = std::make_shared<io_device_client>(get_home(), low_latency);
    reader_ = io_device_->open_reader_to_subscribe();
    os::handle_os_signals();
}

void apprentice::subscribe(const data::location_ptr location)
{
    int64_t now = time::now_in_nano();
    reader_->subscribe(location, 0, now);
    reader_->subscribe(location, get_home()->uid, now);
    SPDLOG_INFO("apprentice subscribed for {}", location->uname);
}

void apprentice::add_event_handler(event_handler_ptr handler)
{
    event_handlers_.push_back(handler);
}

void apprentice::go()
{
    for (auto handler: event_handlers_)
    {
        handler->configure_event_source(shared_from_this());
    }

    hero::go();

    for (auto handler: event_handlers_)
    {
        handler->finish();
    }
    SPDLOG_INFO("apprentice {} finished", get_home()->uname);
}

void apprentice::try_once()
{
    if (io_device_->get_observer()->wait())
    {
        std::string notice = io_device_->get_observer()->get_notice();
        nanomsg_json event(notice);
        for (auto handler : event_handlers_)
        {
            handler->handle(&event);
        }
    }

    if (reader_.get() != nullptr && reader_->data_available())
    {
        for (auto handler : event_handlers_)
        {
            handler->handle(&reader_->current_frame());
        }
        reader_->next();
    }

    if (socket_reply_.get() != nullptr && socket_reply_->recv() > 0)
    {
        SPDLOG_INFO("handle reply nn event {}", socket_reply_->last_message());
        nanomsg_json event(socket_reply_->last_message());
        for (auto handler : event_handlers_)
        {
            handler->handle(&event);
        }
    }

    for (auto element : sub_sockets_)
    {
        if (element.second->recv() > 0)
        {
            SPDLOG_INFO("handle sub socket nn event {}", element.second->last_message());
            nanomsg_json event(element.second->last_message());
            for (auto handler : event_handlers_)
            {
                handler->handle(&event);
            }
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