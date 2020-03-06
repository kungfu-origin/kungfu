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
            apprentice(location::make_shared(m, category::SYSTEM, "service", "ledger", std::move(locator)), low_latency),
            publish_state(state_map_),
            assets_(state_map_[boost::hana::type_c<longfist::types::Asset>])
    {
        log::copy_log_settings(get_io_device()->get_home(), "ledger");
        if (m == mode::LIVE)
        {
            pub_sock_ = get_io_device()->bind_socket(nanomsg::protocol::PUBLISH);
        }
    }

    book::BookContext_ptr Ledger::get_book_context()
    {
        if (book_context_ == nullptr)
        {
            book_context_ = std::make_shared<book::BookContext>(*this, events_);
        }
        return book_context_;
    }

    void Ledger::publish(const std::string &msg)
    {
        if (get_io_device()->get_home()->mode == mode::LIVE)
        {
            pub_sock_->send(msg);
        }
        SPDLOG_DEBUG("published {}", msg);
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

    std::vector<longfist::types::Position> Ledger::get_positions(const yijinjing::data::location_ptr &location)
    {
        auto position_map = state_map_[boost::hana::type_c<longfist::types::Position>];
        std::vector<longfist::types::Position> res = {};
        for (const auto &kv: position_map)
        {
            const auto &position = kv.second.data;
            if (position.holder_uid == location->uid)
            {
                res.push_back(position);
            }
        }
        return res;
    }

    bool Ledger::has_asset(const yijinjing::data::location_ptr &location)
    {
        return assets_.find(location->uid) != assets_.end();
    }

    longfist::types::Asset Ledger::get_asset(const yijinjing::data::location_ptr &location)
    {
        if (assets_.find(location->uid) == assets_.end())
        {
            throw wingchun_error(fmt::format("asset info for {} not exist", location->uname));
        }
        return assets_.at(location->uid).data;
    }

    void Ledger::dump_asset_snapshot(const Asset &asset)
    {
        auto writer = writers_.at(0);
        AssetSnapshot &snapshot = writer->open_data<AssetSnapshot>();
        memcpy(&snapshot, &asset, sizeof(snapshot));
        snapshot.update_time = now();
        writer->close_data();
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

    void Ledger::on_start()
    {
        pre_start();

        events_ | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              auto register_data = event->data<Register>();
              auto app_location = location::make_shared(register_data, get_locator());
              if (app_location->category != category::SYSTEM)
              {
                  request_write_to(event->gen_time(), app_location->uid);
                  request_read_from_public(event->gen_time(), app_location->uid, register_data.checkin_time);
              }
              if (app_location->category == category::MD)
              {
                  update_broker_state(event->gen_time(), app_location, BrokerState::Connected);
                  monitor_market_data(event->gen_time(), app_location->uid);
              }
              if (app_location->category == category::TD)
              {
                  request_read_from(event->gen_time(), app_location->uid, register_data.checkin_time);
                  update_broker_state(event->gen_time(), app_location, BrokerState::Connected);
                  monitor_instruments(app_location->uid);
              }
          });

        events_ | is(Deregister::tag) |
        $([&](const event_ptr &event)
          {
              auto app_location = location::make_shared(event->data<Deregister>(), get_locator());
              if (app_location->category == category::MD or app_location->category == category::TD)
              {
                  update_broker_state(event->gen_time(), app_location, BrokerState::DisConnected);
                  broker_states_.erase(app_location->uid);
              }
          });

        events_ | is(Channel::tag) |
        $([&](const event_ptr &event)
          {
              auto home_uid = get_home_uid();
              const Channel &channel = event->data<Channel>();
              if (channel.source_id != home_uid and channel.dest_id != home_uid)
              {
                  reader_->join(get_location(channel.source_id), channel.dest_id, event->gen_time());
              }
          });

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
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
          });

        events_ | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_order(event, event->data<Order>()); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
          });

        events_ | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_trade(event, event->data<Trade>()); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
          });

        events_ | is(OrderActionError::tag) |
        $([&](const event_ptr &event)
          {
              try
              { on_order_action_error(event, event->data<OrderActionError>()); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
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
                          get_locator()
                  );
                  SPDLOG_INFO("asset for {}[{:08x}] requested", app_location->uname, app_location->uid);
                  handle_asset_request(event, app_location);
              }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
          });

        events_ | is(InstrumentRequest::tag) |
        $([&](const event_ptr &event)
          {
              try
              { handle_instrument_request(event); }
              catch (const std::exception &e)
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
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
              { SPDLOG_ERROR("Unexpected exception {}", e.what()); }
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
              { SPDLOG_WARN("{}", ex.what()); }
              catch (const std::exception &ex)
              { SPDLOG_WARN("Unexpected exception {}", ex.what()); }
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

    void Ledger::request_subscribe(uint32_t account_location_id, const std::vector<Instrument> &instruments)
    {
        if (!has_location(account_location_id))
        {
            SPDLOG_ERROR("failed to find account location [{:08x}]", account_location_id);
            return;
        }
        auto location = get_location(account_location_id);
        auto md_location = location::make_shared(get_io_device()->get_home()->mode, category::MD, location->group, location->group,
                                                 get_locator());
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
                               const BrokerStateUpdate &data = e->data<BrokerStateUpdate>();
                               return data.state == BrokerState::LoggedIn or data.state == BrokerState::Ready;
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

