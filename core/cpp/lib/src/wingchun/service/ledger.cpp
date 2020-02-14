//
// Created by Keren Dong on 2019-06-28.
//

#include <utility>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/utils.h>
#include <kungfu/wingchun/service/ledger.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::service
{
    Ledger::Ledger(locator_ptr locator, mode m, bool low_latency) :
            apprentice(location::make(m, category::SYSTEM, "service", "ledger", std::move(locator)), low_latency),
            publish_state(state_map_)
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

    BrokerState Ledger::get_broker_state(uint32_t broker_location) const
    {
        if (broker_states_.find(broker_location) != broker_states_.end())
        {
            return broker_states_.at(broker_location);
        } else
        {
            return BrokerState::Unknown;
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
                request_read_from(trigger_time, app_location->uid, trigger_time);
                update_broker_state(trigger_time, app_location, BrokerState::Connected);
                monitor_instruments(app_location->uid);
                break;
            }
            case category::STRATEGY:
            {
                watch(trigger_time, app_location);
                request_write_to(trigger_time, app_location->uid);
                break;
            }
            default:
            {
                break;
            }
        }
        on_app_location(trigger_time, app_location);
    }

    void Ledger::deregister_location(int64_t trigger_time, uint32_t location_uid)
    {
        if (not has_location(location_uid))
        {
            return;
        }
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

    void Ledger::on_write_to(const event_ptr &event)
    {
        if (event->source() == get_master_commands_uid())
        {
            apprentice::on_write_to(event);
        }
    }

    void Ledger::on_read_from(const event_ptr &event)
    {
        const RequestReadFrom &request = event->data<RequestReadFrom>();
        auto source_location = get_location(request.source_id);
        auto master_cmd_location = get_location(event->source());

        std::stringstream ss;
        ss << std::hex << master_cmd_location->name;
        uint32_t dest_id;
        ss >> dest_id;
        auto dest_location = get_location(dest_id);

        if (source_location->uid == get_master_commands_uid() or event->dest() == get_live_home_uid())
        {
            apprentice::on_read_from(event);
            return;
        }
        if (event->msg_type() == RequestReadFrom::tag &&
            source_location->category == category::TD && dest_location->category == category::STRATEGY)
        {
            SPDLOG_INFO("ledger read order/trades from {} to {}", source_location->uname, dest_location->uname);
            reader_->join(source_location, dest_id, event->gen_time());
        }
    }

    void Ledger::monitor_instruments(uint32_t broker_location)
    {
        auto insts = events_ | from(broker_location) |
                     take_while([=](const event_ptr &event)
                                { return event->msg_type() != InstrumentEnd::tag; })
                     | is(Instrument::tag) |
                     reduce(std::vector<Instrument>(),
                            [](std::vector<Instrument> res, event_ptr event)
                            {
                                res.push_back(event->data<Instrument>());
                                return res;
                            }) | as_dynamic();
        insts.subscribe(
                [=](std::vector<Instrument> res)
                {
                    if (!this->is_live())
                    { return; }
                    SPDLOG_INFO("instrument info updated from {} [{:08x}], size: {}",
                                has_location(broker_location) ? get_location(broker_location)->uname : "unknown",
                                broker_location, res.size());
                    this->on_instruments(res);
                    this->monitor_instruments(broker_location);
                });
    }

    book::BookContext_ptr Ledger::get_book_context()
    {
        if (book_context_ == nullptr)
        {
            book_context_ = std::make_shared<book::BookContext>(*this, events_);
        }
        return book_context_;
    }

    void Ledger::on_start()
    {
        pre_start();

        events_ | is(BrokerStateUpdate::tag) |
        $([&](const event_ptr &event)
          {
              auto broker_location = get_location(event->source());
              update_broker_state(event->gen_time(), broker_location, static_cast<longfist::enums::BrokerState>(event->data<int32_t>()));
          });

        events_ | is(Position::tag) | filter([&](const event_ptr &event)
                                                   {
                                                       auto source = event->source();
                                                       return this->has_location(source) && this->get_location(source)->category == category::TD;
                                                   }) |
        $([&](const event_ptr &event)
          {
              const auto &position = event->data<Position>();
              auto insts = convert_to_instruments(std::vector<Position>({position}));
              request_subscribe(event->source(), insts);
          });

        /**
         * process trade events
         */
        events_ | is(Quote::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_quote(event, event->data<Quote>()); }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        events_ | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_order(event, event->data<Order>()); }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        events_ | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_trade(event, event->data<Trade>()); }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        events_ | is(OrderActionError::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_order_action_error(event, event->data<OrderActionError>()); }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        events_ | is(QryAsset::tag) |
        $([&](const event_ptr &event)
          {
              try
              {
                  const char *buffer = &(event->data<char>());
                  std::string json_str{};
                  json_str.assign(buffer, event->data_length());
                  nlohmann::json location_json = nlohmann::json::parse(json_str);
                  auto app_location = std::make_shared<location>(
                          static_cast<mode>(location_json["mode"]),
                          static_cast<category>(location_json["category"]),
                          location_json["group"], location_json["name"],
                          get_io_device()->get_home()->locator
                  );
                  SPDLOG_INFO("asset for {}[{:08x}] requested", app_location->uname, app_location->uid);
                  handle_asset_request(event, app_location);
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        events_ | is(InstrumentRequest::tag) |
        $([&](const event_ptr &event)
          {
              try
              {
                  handle_instrument_request(event);
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          });

        /**
         * process active query from clients
         */
        events_ |
        filter([&](const event_ptr &event)
               {
                   return dynamic_cast<nanomsg::nanomsg_json *>(event.get()) != nullptr and event->source() == 0;
               }) |
        $([&](const event_ptr &event)
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

        publish_state(writers_[0], now());

        apprentice::on_start();

    }

    void Ledger::publish_broker_state(int64_t trigger_time, const location_ptr &broker_location, BrokerState state)
    {
        nlohmann::json msg;
        msg["gen_time"] = time::now_in_nano();
        msg["trigger_time"] = trigger_time;
        msg["msg_type"] = BrokerStateUpdate::tag;
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
        if (not has_location(master_cmd_location->uid))
        {
            register_location(trigger_time, master_cmd_location);
        }
        reader_->join(master_cmd_location, app_location->uid, trigger_time);
        reader_->join(app_location, 0, trigger_time);
    }

    void Ledger::monitor_market_data(int64_t trigger_time, uint32_t md_location_uid)
    {
        events_ | from(md_location_uid) | is(Quote::tag) | first() |
        $([&, trigger_time, md_location_uid](const event_ptr &event)
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
        events_ | from(md_location_uid) | is(Quote::tag) | timeout(std::chrono::seconds(5)) |
        $([&](const event_ptr &e)
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

    uint64_t Ledger::cancel_order(const event_ptr &event, uint32_t account_location_uid, uint64_t order_id)
    {
        SPDLOG_INFO("cancel order {}", order_id);
        if (has_writer(account_location_uid))
        {
            auto writer = get_writer(account_location_uid);
            OrderAction &action = writer->open_data<OrderAction>(event->gen_time());
            action.order_action_id = writer->current_frame_uid();
            action.order_id = order_id;
            action.action_flag = OrderActionFlag::Cancel;
            writer->close_data();
            return action.order_action_id;
        } else
        {
            if (has_location(account_location_uid))
            {
                SPDLOG_ERROR("writer to {} [{:08x}] not exists", get_location(account_location_uid)->uname, account_location_uid);
            } else
            {
                SPDLOG_ERROR("writer to [{:08x}] not exists", account_location_uid);
            }
            return 0;
        }
    }

    void Ledger::request_subscribe(uint32_t account_location_id, const std::vector<Instrument> &instruments)
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

        auto sub_util_broker_ready = [=]()
        {
            BrokerState state = this->get_broker_state(md_location->uid);
            switch (state)
            {
                case BrokerState::LoggedIn:
                case BrokerState::Ready:
                {
                    write_sub_msg();
                    break;
                }
                default:
                {
                    events_ | is(BrokerStateUpdate::tag) | from(md_location->uid) |
                    filter([=](const event_ptr &e)
                           {
                               const BrokerState &data = e->data<BrokerState>();
                               return data == BrokerState::LoggedIn or data == BrokerState::Ready;
                           }) | first() |
                    $([=](const event_ptr &e)
                      {
                          write_sub_msg();
                      });
                    break;
                }
            }
        };

        if (not has_writer(md_location->uid))
        {
            events_ | is(RequestWriteTo::tag) |
            filter([=](const event_ptr &e)
                   {
                       const RequestWriteTo &data = e->data<RequestWriteTo>();
                       return data.dest_id == md_location->uid;
                   }) | first() |
            $([=](const event_ptr &e)
              {
                  sub_util_broker_ready();
              });
        } else
        {
            sub_util_broker_ready();
        }
    }
}

