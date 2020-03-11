//
// Created by Keren Dong on 2019-06-20.
//

#include <utility>
#include <hffix.hpp>

#include <kungfu/wingchun/utils.h>
#include <kungfu/wingchun/broker/marketdata.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::broker
{
    MarketData::MarketData(bool low_latency, locator_ptr locator, const std::string &source) :
            Broker(location::make_shared(mode::LIVE, category::MD, source, source, std::move(locator)), low_latency)
    {}

    void MarketData::on_start()
    {
        Broker::on_start();

        events_ | is(SubscribeAll::tag) |
        $([&](const event_ptr &event)
          {
              SPDLOG_INFO("subscribe all request");
              subscribe_all();
          });

        events_ | is(Subscribe::tag) |
        $([&](const event_ptr &event)
          {
              SPDLOG_INFO("subscribe request");
              std::vector<Instrument> symbols;
              const char *buffer = &(event->data<char>());
              hffix::message_reader reader(buffer, buffer + event->data_length());
              for (; reader.is_complete(); reader = reader.next_message_reader())
              {
                  if (reader.is_valid())
                  {
                      auto group_mdentry_begin = std::find_if(reader.begin(), reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));
                      hffix::message_reader::const_iterator group_mdentry_end;
                      for (; group_mdentry_begin != reader.end(); group_mdentry_begin = group_mdentry_end)
                      {
                          group_mdentry_end = std::find_if(group_mdentry_begin + 1, reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));

                          auto group_instrument_begin = std::find_if(group_mdentry_begin, group_mdentry_end,
                                                                     hffix::tag_equal(hffix::tag::Symbol));
                          hffix::message_reader::const_iterator group_instrument_end;

                          for (; group_instrument_begin != group_mdentry_end; group_instrument_begin = group_instrument_end)
                          {
                              group_instrument_end = std::find_if(group_instrument_begin + 1, group_mdentry_end,
                                                                  hffix::tag_equal(hffix::tag::Symbol));
                              hffix::message_reader::const_iterator symbol = group_instrument_begin;
                              hffix::message_reader::const_iterator exchange = group_instrument_begin;
                              reader.find_with_hint(hffix::tag::SecurityExchange, exchange);

                              Instrument instrument{};
                              strcpy(instrument.instrument_id, symbol->value().as_string().c_str());
                              strcpy(instrument.exchange_id, exchange->value().as_string().c_str());
                              symbols.push_back(instrument);
                          }
                      }
                  }
              }
              subscribe(symbols);
          });
    }

    Subscription::Subscription(apprentice &app) : app_(app)
    {
    }

    const std::unordered_map<uint32_t, longfist::types::Instrument> &Subscription::get_instruments() const
    {
        return instruments_;
    }

    void Subscription::add(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id)
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

    void Subscription::add_all(const location_ptr &md_location)
    {
        md_locations_.emplace(md_location->uid, true);
    }

    bool Subscription::is_subscribed(const location_ptr &md_location, const std::string &exchange_id, const std::string &instrument_id) const
    {
        return is_all_subscribed(md_location) or instruments_.find(get_symbol_id(exchange_id.c_str(), instrument_id.c_str())) != instruments_.end();
    }

    bool Subscription::is_all_subscribed(const location_ptr &md_location) const
    {
        return should_subscribe(md_location) and md_locations_.at(md_location->uid);
    }

    void Subscription::subscribe(const rx::connectable_observable<event_ptr> &events)
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

    uint32_t Subscription::hash_instrument(const longfist::types::Instrument &instrument) const
    {
        return get_symbol_id(instrument.exchange_id, instrument.instrument_id);
    }

    bool Subscription::should_subscribe(const yijinjing::data::location_ptr &md_location) const
    {
        return md_locations_.find(md_location->uid) != md_locations_.end() ;
    }

    void Subscription::connect(const longfist::types::Register& register_data)
    {
        auto app_location = app_.get_location(register_data.location_uid);
        if (app_location->category == category::MD and should_subscribe(app_location))
        {
            app_.request_read_from_public(app_.now(), app_location->uid, register_data.checkin_time);
            app_.request_write_to(app_.now(), app_location->uid);
        }
    }
}
