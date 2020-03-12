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
    Client::Client(apprentice &app, bool monitor_all_md, bool monitor_all_td) :
            app_(app), monitor_all_md_(monitor_all_md), monitor_all_td_(monitor_all_td)
    {
    }

    const std::unordered_map<uint32_t, longfist::types::Instrument> &Client::get_instruments() const
    {
        return instruments_;
    }

    void Client::subscribe(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id)
    {
        if (not is_enrolled_md(md_location))
        {
            enrolled_md_locations_.emplace(md_location->uid, false);
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

    void Client::subscribe_all(const location_ptr &md_location)
    {
        enrolled_md_locations_.emplace(md_location->uid, true);
    }

    bool Client::is_subscribed(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id) const
    {
        return is_all_subscribed(md_location) or instruments_.find(get_symbol_id(exchange_id.c_str(), instrument_id.c_str())) != instruments_.end();
    }

    bool Client::is_all_subscribed(const location_ptr &md_location) const
    {
        return is_enrolled_md(md_location) and enrolled_md_locations_.at(md_location->uid);
    }

    void Client::enroll_account(const yijinjing::data::location_ptr &td_location)
    {
        enrolled_td_locations_.emplace(td_location->uid, true);
    }

    bool Client::is_ready(uint32_t broker_location_uid) const
    {
        if (app_.has_location(broker_location_uid) and app_.has_writer(broker_location_uid))
        {
            auto broker_location = app_.get_location(broker_location_uid);
            bool md_test = broker_location->category == category::MD and ready_md_locations_.find(broker_location->uid) != ready_md_locations_.end();
            bool td_test = broker_location->category == category::TD and ready_td_locations_.find(broker_location->uid) != ready_td_locations_.end();
            return md_test or td_test;
        }
        return false;
    }

    void Client::on_start(const rx::connectable_observable<event_ptr> &events)
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
                  bool ready = ready_md_locations_.find(broker_location->uid) != ready_md_locations_.end();
                  if (good_state and not ready and app_.has_writer(broker_location->uid))
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
                      ready_md_locations_.emplace(broker_location->uid, broker_location);
                  }
                  if (state == BrokerState::Connected or state == BrokerState::DisConnected)
                  {
                      ready_md_locations_.erase(broker_location->uid);
                  }
              }
              if (broker_location->category == category::TD)
              {
                  if (state == BrokerState::Ready)
                  {
                      ready_td_locations_.emplace(broker_location->uid, broker_location);
                  }
                  if (state == BrokerState::Connected or state == BrokerState::DisConnected)
                  {
                      ready_td_locations_.erase(broker_location->uid);
                  }
              }
              broker_states_.emplace(broker_location->uid, state);
          });

        events | is(Deregister::tag) |
        $([&](const event_ptr &event)
          {
              auto location_uid = event->data<Deregister>().location_uid;
              broker_states_.emplace(location_uid, BrokerState::DisConnected);
              ready_md_locations_.erase(location_uid);
              ready_td_locations_.erase(location_uid);
          });
    }

    uint32_t Client::hash_instrument(const longfist::types::Instrument &instrument) const
    {
        return get_symbol_id(instrument.exchange_id, instrument.instrument_id);
    }

    bool Client::is_enrolled_md(const yijinjing::data::location_ptr &md_location) const
    {
        return enrolled_md_locations_.find(md_location->uid) != enrolled_md_locations_.end();
    }

    bool Client::is_enrolled_td(const yijinjing::data::location_ptr &td_location) const
    {
        return enrolled_td_locations_.find(td_location->uid) != enrolled_td_locations_.end();
    }

    void Client::connect(const longfist::types::Register &register_data)
    {
        auto app_location = app_.get_location(register_data.location_uid);
        if (app_location->category == category::MD and (monitor_all_md_ or is_enrolled_md(app_location)))
        {
            app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
            app_.request_write_to(app_.now(), app_location->uid);
        }
        if (app_location->category == category::TD and (monitor_all_td_ or is_enrolled_td(app_location)))
        {
            app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
            app_.request_read_from(app_.now(), app_location->uid, register_data.checkin_time);
            app_.request_write_to(app_.now(), app_location->uid);
        }
    }
}

