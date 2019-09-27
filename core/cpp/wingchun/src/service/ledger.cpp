//
// Created by Keren Dong on 2019-06-28.
//

#include <utility>
#include <sstream>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/utils.h>
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
                SPDLOG_DEBUG("published {}", msg);
            }

            msg::data::BrokerState Ledger::get_broker_state(uint32_t broker_location) const
            {
                if (broker_states_.find(broker_location) != broker_states_.end())
                {
                    return broker_states_.at(broker_location);
                } else
                {
                    return msg::data::BrokerState::Unknown;
                }
            }

            void Ledger::publish_broker_states(int64_t trigger_time)
            {
                SPDLOG_DEBUG("publishing broker states");
                for (auto item : broker_states_)
                {
                    if (has_location(item.first))
                    {
                        publish_broker_state(trigger_time, get_location(item.first), item.second);
                    }
                }
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
                        request_write_to(trigger_time, app_location->uid);
                        update_broker_state(trigger_time, app_location, BrokerState::Connected);
                        monitor_market_data(trigger_time, app_location->uid);
                        break;
                    }
                    case category::TD:
                    {
                        watch(trigger_time, app_location);
                        request_write_to(trigger_time, app_location->uid);
                        update_broker_state(trigger_time, app_location, BrokerState::Connected);
                        on_trader_started(trigger_time, app_location);
                        monitor_trader(app_location->uid);
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
                        update_broker_state(trigger_time, app_location, BrokerState::DisConnected);
                        broker_states_.erase(location_uid);
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
                    SPDLOG_INFO("ledger read order/trades from {} to {}", source_location->uname, dest_location->uname);
                    reader_->join(source_location, dest_id, event->gen_time());
                }
            }

            void Ledger::on_start()
            {
                apprentice::on_start();

                pre_start();

                events_ | is(msg::type::BrokerState) |
                $([&](event_ptr event)
                  {
                      auto broker_location = get_location(event->source());
                      update_broker_state(event->gen_time(), broker_location, static_cast<BrokerState>(event->data<int32_t>()));
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

                /**
                 * process active query from clients
                 */
                events_ |
                filter([&](event_ptr event)
                       {
                           return dynamic_cast<nanomsg::nanomsg_json *>(event.get()) != nullptr and event->source() == 0;
                       }) |
                $([&](event_ptr event)
                  {
                      // let python do the actual job, we just operate the I/O part
                      try
                      {
                          const nlohmann::json &cmd = event->data<nlohmann::json>();
                          SPDLOG_INFO("handle command type {} data {}", event->msg_type(), cmd.dump());
                          std::string response = handle_request(event, event->to_string());
                          get_io_device()->get_rep_sock()->send(response);
                      }
                      catch (const std::exception &e)
                      {
                          SPDLOG_ERROR("Unexpected exception {}", e.what());
                      }
                  });
            }

            void Ledger::publish_broker_state(int64_t trigger_time, const location_ptr &broker_location, BrokerState state)
            {
                nlohmann::json msg;
                msg["gen_time"] = time::now_in_nano();
                msg["trigger_time"] = trigger_time;
                msg["msg_type"] = msg::type::BrokerState;
                msg["source"] = get_io_device()->get_home()->uid;

                nlohmann::json data;
                data["category"] = broker_location->category;
                data["group"] = broker_location->group;
                data["name"] = broker_location->name;
                data["state"] = state;
                msg["data"] = data;
                publish(msg.dump());
            }

            void Ledger::update_broker_state(int64_t trigger_time, const location_ptr &broker_location, BrokerState state)
            {
                broker_states_[broker_location->uid] = state;
                publish_broker_state(trigger_time, broker_location, state);
            }

            void Ledger::watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
            {
                auto app_uid_str = fmt::format("{:08x}", app_location->uid);
                auto master_cmd_location = location::make(mode::LIVE, category::SYSTEM, "master", app_uid_str, app_location->locator);
                reader_->join(master_cmd_location, app_location->uid, trigger_time);
                reader_->join(app_location, 0, trigger_time);
            }

            void Ledger::monitor_trader(uint32_t td_location_uid)
            {
                auto insts = events_| from(td_location_uid) | take_while([&](event_ptr event) { return event->msg_type() != msg::type::InstrumentEnd;}) |
                             is(msg::type::Instrument) |
                             reduce(std::vector<Instrument>(),
                                    [](std::vector<Instrument> res, event_ptr event)
                                    {
                                        res.push_back(event->data<msg::data::Instrument>());
                                        return res;
                                    }) | as_dynamic();

                insts.subscribe([&](std::vector<Instrument> res)
                {
                    on_instruments(res);
                });

                auto asset = events_ | from(td_location_uid) | is(msg::type::Asset) | first();

                auto positions = events_ | from(td_location_uid) |
                        take_while([&](event_ptr event) { return event->msg_type() != msg::type::PositionEnd;}) |
                        is(msg::type::Position) | as_dynamic();

                auto position_details = events_ | from(td_location_uid) | take_while([&](event_ptr event) { return event->msg_type() != msg::type::PositionDetailEnd;}) |
                        is(msg::type::PositionDetail) | as_dynamic();

                asset.merge(positions).reduce(std::make_pair(Asset(), std::vector<Position>({})),
                        [](std::pair<Asset, std::vector<Position>> res, event_ptr event)
                        {
                            if (event->msg_type() == msg::type::Asset) { res.first = event->data<Asset>(); }
                            else if(event->msg_type() == msg::type::Position) { res.second.push_back(event->data<Position>()); }
                            return res;
                        }).subscribe(
                                [&, td_location_uid](std::pair<Asset, std::vector<Position>> res)
                                {
                                    on_stock_account(res.first, res.second);
                                    request_subscribe(td_location_uid, convert_to_instruments(res.second));
                                    monitor_trader(td_location_uid);
                                });

                asset.merge(position_details).reduce(std::make_pair(Asset(), std::vector<PositionDetail>({})),
                                              [](std::pair<Asset, std::vector<PositionDetail>> res, event_ptr event)
                                              {
                                                  if (event->msg_type() == msg::type::Asset) { res.first = event->data<Asset>(); }
                                                  else if(event->msg_type() == msg::type::Position) { res.second.push_back(event->data<PositionDetail>()); }
                                                  return res;
                                              }).subscribe(
                        [&, td_location_uid](std::pair<Asset, std::vector<PositionDetail>> res)
                        {
                            on_future_account(res.first, res.second);
                            request_subscribe(td_location_uid, convert_to_instruments(res.second));
                            monitor_trader(td_location_uid);
                        });
            }

            void Ledger::monitor_market_data(int64_t trigger_time, uint32_t md_location_uid)
            {
                events_ | from(md_location_uid) | is(msg::type::Quote) | first() |
                $([&, trigger_time, md_location_uid](event_ptr event)
                  {
                      if (has_location(md_location_uid))
                      {
                          auto md_location = get_location(md_location_uid);
                          update_broker_state(trigger_time, md_location, BrokerState::Ready);
                          alert_market_data(trigger_time, md_location_uid);
                      }
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
                      if (has_location(md_location_uid))
                      {
                          auto md_location = get_location(md_location_uid);
                          update_broker_state(trigger_time, md_location, BrokerState::Idle);
                          monitor_market_data(trigger_time, md_location_uid);
                      }
                  });
            }

            void Ledger::new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input)
            {}

            void Ledger::cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id)
            {
                SPDLOG_INFO("cancel order {}", order_id);
                if (has_writer(account_location_uid))
                {
                    auto writer = get_writer(account_location_uid);
                    msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(event->gen_time(), msg::type::OrderAction);
                    action.order_action_id = writer->current_frame_uid();
                    action.order_id = order_id;
                    action.action_flag = OrderActionFlag::Cancel;
                    writer->close_data();
                } else
                {
                    if (has_location(account_location_uid))
                    {
                        auto location = get_location(account_location_uid);
                        SPDLOG_ERROR("writer to {} [{:08x}] not exists", location->uname, account_location_uid);
                    } else
                    {
                        SPDLOG_ERROR("writer to [{:08x}] not exists", account_location_uid);
                    }
                }
            }

            void Ledger::request_subscribe(uint32_t account_location_id, const std::vector<msg::data::Instrument> &&instruments)
            {
                if (!has_location(account_location_id))
                {
                    SPDLOG_ERROR("failed to find account location [{:08x}]", account_location_id);
                    return;
                }
                auto location = get_location(account_location_id);
                auto md_location = location::make(get_io_device()->get_home()->mode, category::MD, location->group, location->group,
                                                  get_io_device()->get_home()->locator);
                SPDLOG_INFO("subscribe {} instruments for account {}@{} from {} [{:08x}]",
                        instruments.size(), location->name, location->group, md_location->uname, md_location->uid);

                auto write_sub_msg = [=]()
                {
                    for (const auto &instrument : instruments)
                    {
                        write_subscribe_msg(this->get_writer(md_location->uid), now(), instrument.exchange_id, instrument.instrument_id);
                    }
                };

                msg::data::BrokerState state = this->get_broker_state(md_location->uid);
                switch (state)
                {
                    case msg::data::BrokerState::LoggedIn:
                    case msg::data::BrokerState::Ready:
                    {
                        write_sub_msg();
                        break;
                    }
                    default:
                    {
                        events_ | is(msg::type::BrokerState) | from(md_location->uid) |
                        filter([=](yijinjing::event_ptr e)
                               {
                                   const msg::data::BrokerState &data = e->data<msg::data::BrokerState>();
                                   return data == msg::data::BrokerState::LoggedIn or data == msg::data::BrokerState::Ready;
                               }) | first() |
                        $([=](event_ptr e)
                          {
                              write_sub_msg();
                          });
                        break;
                    }
                }
            }
        }
    }
}

