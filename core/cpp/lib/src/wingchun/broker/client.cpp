//
// Created by Keren Dong on 2020/3/12.
//

#include <utility>

#include <kungfu/wingchun/utils.h>
#include <kungfu/wingchun/broker/client.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker
{
    Client::Client(apprentice &app) : app_(app)
    {
    }

    const std::unordered_map<uint32_t, longfist::types::Instrument> &Client::get_instruments() const
    {
        return instruments_;
    }

    void Client::add(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id)
    {
        if (not should_subscribe(md_location))
        {
            md_locations_.emplace(md_location->uid, false);
        }
        if (not is_all_subscribed(md_location))
        {
            Instrument instrument = {};
            strcpy(instrument.instrument_id, instrument_id.c_str());
            strcpy(instrument.exchange_id, exchange_id.c_str());
            uint32_t key = hash_instrument(instrument);
            instruments_.emplace(key, instrument);
            instrument_md_locations_.emplace(key, md_location);
        }
    }

    void Client::add_all(const location_ptr &md_location)
    {
        md_locations_.emplace(md_location->uid, true);
    }

    bool Client::is_subscribed(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id) const
    {
        return is_all_subscribed(md_location) or instruments_.find(get_symbol_id(exchange_id.c_str(), instrument_id.c_str())) != instruments_.end();
    }

    bool Client::is_all_subscribed(const location_ptr &md_location) const
    {
        return should_subscribe(md_location) and md_locations_.at(md_location->uid);
    }

    void Client::subscribe(const rx::connectable_observable<event_ptr> &events)
    {
        events | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              connect(event->data<Register>());
          });

        events | is(BrokerStateUpdate::tag) |
        $([&](const event_ptr &event)
          {
              auto state = event->data<BrokerStateUpdate>().state;
              auto broker_location = app_.get_location(event->source());
              if (broker_location->category == category::MD)
              {
                  bool good_state = state == BrokerState::LoggedIn or state == BrokerState::Ready;
                  bool subscribed = subscribed_md_locations_.find(broker_location->uid) != subscribed_md_locations_.end();
                  if (good_state and not subscribed and app_.has_writer(broker_location->uid))
                  {
                      auto writer = app_.get_writer(broker_location->uid);
                      if (is_all_subscribed(broker_location))
                      {
                          SPDLOG_INFO("subscribe all from MD {}", broker_location->uname);
                          writer->mark(event->gen_time(), SubscribeAll::tag);
                      } else
                      {
                          for (auto &pair : instruments_)
                          {
                              auto instrument = pair.second;
                              auto md_location = instrument_md_locations_.at(hash_instrument(instrument));
                              if (md_location->uid == broker_location->uid)
                              {
                                  write_subscribe_msg(writer, event->gen_time(), instrument.exchange_id, instrument.instrument_id);
                              }
                          }
                      }
                      subscribed_md_locations_.emplace(broker_location->uid, broker_location);
                  }
                  if (state == BrokerState::Connected or state == BrokerState::DisConnected)
                  {
                      subscribed_md_locations_.erase(broker_location->uid);
                  }
              }
              broker_states_.emplace(broker_location->uid, state);
          });

        events | is(Deregister::tag) |
        $([&](const event_ptr &event)
          {
              auto location_uid = event->data<Deregister>().location_uid;
              broker_states_.emplace(location_uid, BrokerState::DisConnected);
              subscribed_md_locations_.erase(location_uid);
          });
    }

    uint32_t Client::hash_instrument(const longfist::types::Instrument &instrument) const
    {
        return get_symbol_id(instrument.exchange_id, instrument.instrument_id);
    }

    bool Client::should_subscribe(const yijinjing::data::location_ptr &md_location) const
    {
        return md_locations_.find(md_location->uid) != md_locations_.end() ;
    }

    void Client::connect(const longfist::types::Register& register_data)
    {
        auto app_location = app_.get_location(register_data.location_uid);
        if (app_location->category == category::MD and should_subscribe(app_location))
        {
            app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
            app_.request_write_to(app_.now(), app_location->uid);
        }
    }
}

