//
// Created by Keren Dong on 2019-06-28.
//

#include <utility>
#include <sstream>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/service/ledger.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            Ledger::Ledger(locator_ptr locator, mode m, bool low_latency) :
                    apprentice(location::make(m, category::SYSTEM, "service", "ledger", std::move(locator)), low_latency)
            {
                log::copy_log_settings(get_io_device()->get_home(), "ledger");
                if (m == mode::LIVE)
                {
                    pub_sock_ = get_io_device()->bind_socket(nanomsg::protocol::PUBLISH);
                }
            }

            void Ledger::publish(const std::string &msg)
            {
                if (get_io_device()->get_home()->mode == mode::LIVE)
                {
                    pub_sock_->send(msg);
                }
                SPDLOG_TRACE("published {}", msg);
            }

            void Ledger::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
            {
                if (has_location(app_location->uid))
                {
                    // bypass location events from others master cmd journal
                    return;
                }
                apprentice::register_location(trigger_time, app_location);
                switch (app_location->category)
                {
                    case category::MD:
                    {
                        watch(trigger_time, app_location);
                        update_and_publish_state(trigger_time, app_location, GatewayState::Connected);
                        monitor_market_data(trigger_time, app_location->uid);
                        break;
                    }
                    case category::TD:
                    {
                        watch(trigger_time, app_location);
                        request_write_to(trigger_time, app_location->uid);
                        update_and_publish_state(trigger_time, app_location, GatewayState::Connected);
                        break;
                    }
                    case category::STRATEGY:
                    {
                        watch(trigger_time, app_location);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            void Ledger::deregister_location(int64_t trigger_time, uint32_t location_uid)
            {
                auto app_location = get_location(location_uid);
                switch (app_location->category)
                {
                    case category::MD:
                    case category::TD:
                    {
                        update_and_publish_state(trigger_time, app_location, GatewayState::DisConnected);
                        break;
                    }
                    case category::STRATEGY:
                    {
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                apprentice::deregister_location(trigger_time, location_uid);
            }

            void Ledger::on_write_to(const yijinjing::event_ptr &event)
            {
                if (event->source() == get_master_commands_uid())
                {
                    apprentice::on_write_to(event);
                }
            }

            void Ledger::on_read_from(const yijinjing::event_ptr &event)
            {
                const yijinjing::msg::data::RequestReadFrom &request = event->data<yijinjing::msg::data::RequestReadFrom>();
                auto source_location = get_location(request.source_id);
                auto master_cmd_location = get_location(event->source());

                std::stringstream ss;
                ss << std::hex << master_cmd_location->name;
                uint32_t dest_id;
                ss >> dest_id;
                auto dest_location = get_location(dest_id);

                if (source_location->uid == get_master_commands_uid())
                {
                    apprentice::on_read_from(event);
                    return;
                }
                if (event->msg_type() == yijinjing::msg::type::RequestReadFrom &&
                    source_location->category == category::TD && dest_location->category == category::STRATEGY)
                {
                    SPDLOG_INFO("watcher read order/trades from {} to {}", source_location->uname, dest_location->uname);
                    reader_->join(source_location, dest_id, event->gen_time());
                }
            }

            void Ledger::on_start()
            {
                apprentice::on_start();

                pre_start();

                events_ | is(msg::type::GatewayStateRefresh) |
                $([&](event_ptr event)
                  {
                      publish_all_states(event->gen_time());

                      nlohmann::json msg;
                      msg["status"] = 200;
                      msg["msg_type"] = msg::type::GatewayStateRefresh;
                      get_io_device()->get_rep_sock()->send(msg.dump());
                  });

                events_ | is(msg::type::GatewayState) |
                $([&](event_ptr event)
                  {
                      auto gateway_location = get_location(event->source());
                      update_and_publish_state(event->gen_time(), gateway_location, static_cast<GatewayState>(event->data<int32_t>()));
                  });

                /**
                 * process trade events
                 */
                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      try
                      { on_quote(event, event->data<Quote>()); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                  });

                events_ | is(msg::type::Order) |
                $([&](event_ptr event)
                  {
                      try
                      { on_order(event, event->data<Order>()); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                  });

                events_ | is(msg::type::Trade) |
                $([&](event_ptr event)
                  {
                      try
                      { on_trade(event, event->data<Trade>()); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                  });

                events_ | is(msg::type::Instrument) |
                $([&](event_ptr event)
                  {
                      auto &instrument_buffer = instrument_buffer_[event->source()];
                      instrument_buffer.push_back(event->data<Instrument>());
                  });

                events_ | is(msg::type::InstrumentEnd) |
                $([&](event_ptr event)
                  {
                      auto &instrument_buffer = instrument_buffer_[event->source()];
                      try
                      { on_instruments(instrument_buffer); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                      instrument_buffer.clear();
                  });

                events_ | is(msg::type::Asset) |
                $([&](event_ptr event)
                  {
                      asset_info_[event->source()] = event->data<Asset>();
                  });

                events_ | is(msg::type::Position) |
                $([&](event_ptr event)
                  {
                      auto &position_buffer = position_buffer_[event->source()];
                      position_buffer.push_back(event->data<Position>());
                  });

                events_ | is(msg::type::PositionDetail) |
                $([&](event_ptr event)
                  {
                      auto &buffer = position_detail_buffer_[event->source()];
                      buffer.push_back(event->data<PositionDetail>());
                  });

                events_ | is(msg::type::PositionEnd) |
                $([&](event_ptr event)
                  {
                      auto &asset_info = asset_info_[event->source()];
                      auto &position_buffer = position_buffer_[event->source()];
                      try
                      { on_stock_account(asset_info, position_buffer); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                      position_buffer.clear();
                      memset(&asset_info, 0, sizeof(asset_info));
                  });

                events_ | is(msg::type::PositionDetailEnd) |
                $([&](event_ptr event)
                  {
                      auto &asset_info = asset_info_[event->source()];
                      auto &buffer = position_detail_buffer_[event->source()];
                      try
                      { on_future_account(asset_info, buffer); }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                      buffer.clear();
                      memset(&asset_info, 0, sizeof(asset_info));
                  });
            }

            void Ledger::publish_state(int64_t trigger_time, const location_ptr &gateway_location, GatewayState state)
            {
                nlohmann::json msg;
                msg["gen_time"] = time::now_in_nano();
                msg["trigger_time"] = trigger_time;
                msg["msg_type"] = msg::type::GatewayState;
                msg["source"] = get_io_device()->get_home()->uid;

                nlohmann::json data;
                data["category"] = gateway_location->category;
                data["group"] = gateway_location->group;
                data["name"] = gateway_location->name;
                data["state"] = state;
                msg["data"] = data;
                publish(msg.dump());
            }

            void Ledger::update_and_publish_state(int64_t trigger_time, const location_ptr &gateway_location, GatewayState state)
            {
                gateway_states_[gateway_location->uid] = state;
                publish_state(trigger_time, gateway_location, state);
            }

            void Ledger::publish_all_states(int64_t trigger_time)
            {
                SPDLOG_INFO("publishing gateway states");
                for (auto item : gateway_states_)
                {
                    publish_state(trigger_time, get_location(item.first), item.second);
                }
            }

            void Ledger::watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
            {
                auto app_uid_str = fmt::format("{:08x}", app_location->uid);
                auto master_cmd_location = location::make(mode::LIVE, category::SYSTEM, "master", app_uid_str, app_location->locator);
                reader_->join(master_cmd_location, app_location->uid, trigger_time);
                reader_->join(app_location, 0, trigger_time);
            }

            void Ledger::monitor_market_data(int64_t trigger_time, uint32_t md_location_uid)
            {
                events_ | from(md_location_uid) | is(msg::type::Quote) | first() |
                $([&, trigger_time, md_location_uid](event_ptr event)
                  {
                      auto md_location = get_location(md_location_uid);
                      update_and_publish_state(trigger_time, md_location, GatewayState::Ready);
                      alert_market_data(trigger_time, md_location_uid);
                  },
                  [&](std::exception_ptr e)
                  {
                      try
                      { std::rethrow_exception(e); }
                      catch (const rx::empty_error &ex)
                      {
                          SPDLOG_WARN("{}", ex.what());
                      }
                      catch (const std::exception &ex)
                      {
                          SPDLOG_WARN("Unexpected exception {}", ex.what());
                      }
                  });
            }

            void Ledger::alert_market_data(int64_t trigger_time, uint32_t md_location_uid)
            {
                events_ | from(md_location_uid) | is(msg::type::Quote) | timeout(std::chrono::seconds(5)) |
                $([&](event_ptr e)
                  {},
                  [&, trigger_time, md_location_uid](std::exception_ptr e)
                  {
                      auto md_location = get_location(md_location_uid);
                      update_and_publish_state(trigger_time, md_location, GatewayState::Idle);
                      monitor_market_data(trigger_time, md_location_uid);
                  });
            }

            void Ledger::new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input)
            {}

            void Ledger::cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id)
            {
                auto writer = get_writer(account_location_uid);
                msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(event->gen_time(), msg::type::OrderAction);
                action.order_action_id = writer->current_frame_uid();
                action.order_id = order_id;
                action.action_flag = OrderActionFlag::Cancel;
                writer->close_data();
            }
        }
    }
}

