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

#include <signal.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include <kungfu/practice/apprentice.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;

namespace kungfu
{
    namespace practice
    {
        apprentice::apprentice(location_ptr home, bool low_latency) : hero(std::make_shared<io_device_client>(home, low_latency))
        {
            auto now = time::now_in_nano();
            nlohmann::json request;
            request["msg_type"] = msg::type::Register;
            request["gen_time"] = now;
            request["trigger_time"] = now;
            request["source"] = get_home_uid();

            nlohmann::json data;
            data["mode"] = home->mode;
            data["category"] = home->category;
            data["group"] = home->group;
            data["name"] = home->name;
            data["uid"] = home->uid;
#ifdef _WINDOWS
            data["pid"] = _getpid();
#else
            data["pid"] = getpid();
#endif

            request["data"] = data;

            auto uid_str = fmt::format("{:08x}", get_home_uid());
            auto locator = get_io_device()->get_home()->locator;
            master_commands_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "master", uid_str, locator);
            register_location(master_commands_location_);
            SPDLOG_DEBUG("register master location {} {:08x}", master_commands_location_->uname, master_commands_location_->uid);

            config_location_ = std::make_shared<location>(mode::LIVE, category::SYSTEM, "etc", "kungfu", locator);

            SPDLOG_DEBUG("request {}", request.dump());
            get_io_device()->get_publisher()->publish(request.dump());
        }

        void apprentice::observe(const location_ptr &location, int64_t from_time)
        {
            reader_->join(location, 0, from_time);
        }

        void apprentice::request_write_to(int64_t trigger_time, uint32_t dest_id)
        {
            require_write_to(master_commands_location_->uid, trigger_time, dest_id);
        }

        void apprentice::request_read_from(int64_t trigger_time, uint32_t source_id)
        {
            require_read_from(master_commands_location_->uid, trigger_time, source_id);
        }

        void apprentice::react(const observable<event_ptr> &events)
        {
            events | skip_until(events | is(msg::type::Register) | from(get_home_uid())) | first() | timeout(seconds(1), observe_on_new_thread()) |
            $([&](event_ptr e)
              {
                  // timeout happens on new thread, can not subscribe journal reader here
                  // TODO find a better approach to timeout (use timestamp in journal rather than rx scheduler)
              },
              [&](std::exception_ptr e)
              {
                  try
                  { std::rethrow_exception(e); }
                  catch (const timeout_error &ex)
                  {
                      SPDLOG_ERROR("app register timeout");
                      hero::signal_stop();
                  }
              },
              [&]()
              {
                  // once registered this subscriber finished, no worry for performance.
              });

            events | skip_until(events | is(msg::type::Register) | from(get_home_uid())) | first() |
            $([&](event_ptr e)
              {
                  reader_->join(master_commands_location_, get_home_uid(), e->gen_time());
              });

            events | is(msg::type::Register) |
            $([&](event_ptr e)
              {
                  auto request_loc = e->data<nlohmann::json>();
                  auto app_location = std::make_shared<location>(
                          static_cast<mode>(request_loc["mode"]),
                          static_cast<category>(request_loc["category"]),
                          request_loc["group"], request_loc["name"],
                          get_io_device()->get_home()->locator
                  );
                  register_location(app_location);
                  SPDLOG_INFO("registered location {}", app_location->uname);
              });

            events | is(msg::type::Deregister) |
            $([&](event_ptr e)
              {
                  auto app_location = get_location(e->source());
                  deregister_location(e->source());
                  reader_->disjoin(e->source());
                  writers_.erase(e->source());
                  SPDLOG_INFO("deregistered location {}", app_location->uname);
              });

            events | is(msg::type::RequestWriteTo) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestWriteTo &request = e->data<msg::data::RequestWriteTo>();
                  if (writers_.find(request.dest_id) == writers_.end())
                  {
                      writers_[request.dest_id] = get_io_device()->open_writer(request.dest_id);
                      if (request.dest_id == 0)
                      {
                          writers_[request.dest_id]->open_session();
                      }
                  } else
                  {
                      SPDLOG_ERROR("{} [{:08x}] asks publish to {} [{:08x}] for more than once", get_location(e->source())->uname, e->source(),
                                   get_location(request.dest_id)->uname, request.dest_id);
                  }
              });

            events | is(msg::type::RequestReadFrom) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestReadFrom &request = e->data<msg::data::RequestReadFrom>();
                  SPDLOG_INFO("{} [{:08x}] asks observe at {} [{:08x}] {} from {}", get_location(e->source())->uname, e->source(),
                              get_location(request.source_id)->uname, request.source_id, time::strftime(e->gen_time()),
                              time::strftime(request.from_time));
                  reader_->join(get_location(request.source_id), get_home_uid(), request.from_time);
              });

            events | is(msg::type::RequestStart) | first() |
            $([&](event_ptr e)
              {
                  start();
              });
        }
    }
}

int main(int argc, char **argv)
{
    int64_t n = time::now_in_nano();
    std::string s = time::strftime(n);
    int64_t p = time::strptime(s);
    std::string c = time::strftime(n);
    int32_t len = 28;
    uint32_t ul = len;
    uintptr_t addr = reinterpret_cast<uintptr_t>(&len);
    std::cout << fmt::format("int32_t  = {}", len) << std::endl;
    std::cout << fmt::format("uint32_t = {}", ul) << std::endl;
    std::cout << fmt::format("addr     = {}", addr) << std::endl;
    std::cout << fmt::format("+int32_t = {}", addr + len) << std::endl;
    std::cout << fmt::format("+uint32_t= {}", addr + ul) << std::endl;

    event_ptr e = std::make_shared<nanomsg::nanomsg_json>("{\"data\":{\"value\":1}}");
    std::cout << typeid(event_ptr).name() << std::endl;
    std::cout << typeid(e.get()).name() << std::endl;
    std::cout << typeid(nanomsg::nanomsg_json).name() << std::endl;
    std::cout << typeid(nanomsg::nanomsg_json_ptr).name() << std::endl;
    std::cout << (typeid(e) == typeid(nanomsg::nanomsg_json_ptr)) << std::endl;
    std::cout << (dynamic_cast<nanomsg::nanomsg_json*>(e.get()) == nullptr) << std::endl;
    std::cout << (dynamic_cast<journal::frame*>(e.get()) == nullptr) << std::endl;
    std::cout << e->to_string() << std::endl;
    std::cout << e->data<nlohmann::json>().dump() << std::endl;

    //    char buffer[1024];
//    hffix::message_writer sub_msg(buffer, buffer + 1024);
//    sub_msg.push_back_header("FIX.4.2");
//    sub_msg.push_back_string    (hffix::tag::MsgType, "V"); // Market Data Request
//
//    sub_msg.push_back_int(hffix::tag::MDReqID, 1);
//    sub_msg.push_back_int(hffix::tag::SubscriptionRequestType, 1); // Snapshot + Updates (Subscribe)
//    sub_msg.push_back_int(hffix::tag::MarketDepth, 1);
//
//    sub_msg.push_back_int(hffix::tag::NoMDEntryTypes, 1);
//    sub_msg.push_back_int(hffix::tag::MDEntryType, 2);
//    sub_msg.push_back_int(hffix::tag::NoRelatedSym, 2);
//    sub_msg.push_back_string(hffix::tag::Symbol, "1234");
//    sub_msg.push_back_string(hffix::tag::SecurityExchange, "SSE");
//    sub_msg.push_back_string(hffix::tag::Symbol, "5678");
//    sub_msg.push_back_string(hffix::tag::SecurityExchange, "SSE");
//    sub_msg.push_back_trailer();
//    printf("%s\n", buffer);
//
//    hffix::message_reader reader(buffer, buffer + 1024);
//    for (; reader.is_complete(); reader = reader.next_message_reader())
//    {
//        if (reader.is_valid())
//        {
//            auto group_mdentry_begin = std::find_if(reader.begin(), reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));
//            hffix::message_reader::const_iterator group_mdentry_end;
//            for (; group_mdentry_begin != reader.end(); group_mdentry_begin = group_mdentry_end) {
//                group_mdentry_end = std::find_if(group_mdentry_begin + 1, reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));
//
//                auto group_instrument_begin = std::find_if(group_mdentry_begin, group_mdentry_end, hffix::tag_equal(hffix::tag::Symbol));
//                hffix::message_reader::const_iterator group_instrument_end;
//
//                for (; group_instrument_begin != group_mdentry_end; group_instrument_begin = group_instrument_end) {
//                    group_instrument_end = std::find_if(group_instrument_begin + 1, group_mdentry_end, hffix::tag_equal(hffix::tag::Symbol));
//                    hffix::message_reader::const_iterator symbol = group_instrument_begin;
//                    hffix::message_reader::const_iterator exchange = group_instrument_begin;
//                    reader.find_with_hint(hffix::tag::SecurityExchange, exchange);
//                    SPDLOG_INFO("{} - {}", symbol->value().as_string(), exchange->value().as_string());
//                }
//            }
//        }
//    }
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