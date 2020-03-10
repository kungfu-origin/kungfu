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
            assets_(state_map_[boost::hana::type_c<longfist::types::Asset>]),
            order_stats_(state_map_[boost::hana::type_c<longfist::types::OrderStat>])
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

    uint64_t Ledger::cancel_order(const event_ptr &event, uint32_t account_location_uid, uint64_t order_id)
    {
        if (has_writer(account_location_uid))
        {
            SPDLOG_INFO("cancel order {}", order_id);
            auto writer = get_writer(account_location_uid);
            OrderAction &action = writer->open_data<OrderAction>(event->gen_time());
            action.order_action_id = writer->current_frame_uid();
            action.order_id = order_id;
            action.action_flag = OrderActionFlag::Cancel;
            writer->close_data();
            return action.order_action_id;
        }
        return 0;
    }

    std::vector<longfist::types::Position> Ledger::get_positions(const yijinjing::data::location_ptr &location)
    {
        std::vector<longfist::types::Position> res = {};
        for (const auto &pair: state_map_[boost::hana::type_c<longfist::types::Position>])
        {
            if (pair.second.data.holder_uid == location->uid)
            {
                res.push_back(pair.second.data);
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
        return assets_.at(location->uid).data;
    }

    const std::unordered_map<uint64_t, longfist::types::Instrument> &Ledger::get_instruments() const
    {
        return instruments_;
    }

    void Ledger::dump_asset_snapshot(const Asset &asset)
    {
        auto writer = get_writer(location::PUBLIC);
        AssetSnapshot &snapshot = writer->open_data<AssetSnapshot>();
        memcpy(&snapshot, &asset, sizeof(snapshot));
        snapshot.update_time = now();
        writer->close_data();
    }

    void Ledger::on_ready()
    {
        events_ | is(Position::tag) |
        $([&](const event_ptr &event)
          {
              const Position &position = event->data<Position>();
              auto holder_location = get_location(position.holder_uid);
              if (holder_location->category == category::TD)
              {
                  Instrument instrument = {};
                  strcpy(instrument.instrument_id, position.instrument_id);
                  strcpy(instrument.exchange_id, position.exchange_id);

                  if (instruments_.find(instrument.uid()) == instruments_.end())
                  {
                      auto group = holder_location->group;
                      auto md_location = location::make_shared(holder_location->mode, category::MD, group, group, get_locator());
                      instruments_.emplace(instrument.uid(), instrument);
                      instrument_md_locations_.emplace(instrument.uid(), md_location);
                  }
              }
          });
    }

    void Ledger::on_start()
    {
        pre_start();

        events_ | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              auto register_data = event->data<Register>();
              auto app_location = get_location(register_data.location_uid);
              if (app_location->category != category::SYSTEM)
              {
                  request_read_from_public(event->gen_time(), app_location->uid, register_data.checkin_time);
                  request_read_from(event->gen_time(), app_location->uid, register_data.checkin_time);
                  request_write_to(event->gen_time(), app_location->uid);
              }
          });

        events_ | is(Deregister::tag) |
        $([&](const event_ptr &event)
          {
              auto location_uid = event->data<Deregister>().location_uid;
              broker_states_.emplace(location_uid, BrokerState::DisConnected);
              subscribed_md_locations_.erase(location_uid);
          });

        events_ | is(Channel::tag) |
        $([&](const event_ptr &event)
          {
              const Channel &channel = event->data<Channel>();
              if (channel.source_id != get_home_uid() and channel.dest_id != get_home_uid())
              {
                  reader_->join(get_location(channel.source_id), channel.dest_id, event->gen_time());
              }
          });

        events_ | is(BrokerStateUpdate::tag) |
        $([&](const event_ptr &event)
          {
              auto state = event->data<BrokerStateUpdate>().state;
              auto broker_location = get_location(event->source());
              if (broker_location->category == category::MD)
              {
                  bool good_state = state == BrokerState::LoggedIn or state == BrokerState::Ready;
                  bool subscribed = subscribed_md_locations_.find(broker_location->uid) != subscribed_md_locations_.end();
                  if (good_state and not subscribed and has_writer(broker_location->uid))
                  {
                      SPDLOG_INFO("subscribe from MD {}", broker_location->uname);
                      for (auto &pair : instruments_)
                      {
                          auto instrument = pair.second;
                          auto md_location = instrument_md_locations_.at(instrument.uid());
                          auto writer = get_writer(md_location->uid);
                          if (md_location->uid == broker_location->uid)
                          {
                              write_subscribe_msg(writer, now(), instrument.exchange_id, instrument.instrument_id);
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

        events_ | is(OrderInput::tag) |
        $([&](const event_ptr &event)
          {
              book_context_->on_order_input(event, event->data<OrderInput>());

              OrderStat stat = {};
              stat.order_id = event->data<OrderInput>().order_id;
              stat.md_time = event->trigger_time();
              stat.insert_time = event->gen_time();
              order_stats_.emplace(stat.order_id, state<OrderStat>(get_home_uid(), event->dest(), event->gen_time(), stat));
              write_to(event->gen_time(), stat, event->dest());
          });

        events_ | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              book_context_->on_order(event, event->data<Order>());

              const Order &data = event->data<Order>();
              if (order_stats_.find(data.order_id) == order_stats_.end())
              {
                  order_stats_.emplace(data.order_id, state<OrderStat>(get_home_uid(), event->source(), event->gen_time(), OrderStat()));
              }
              OrderStat &stat = order_stats_.at(data.order_id).data;
              stat.ack_time = event->gen_time();
              write_to(event->gen_time(), stat, event->source());
          });

        events_ | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              book_context_->on_trade(event, event->data<Trade>());
          });

        events_ | is(TradingDay::tag) |
        $([&](const event_ptr &event)
          {
              book_context_->on_trading_day(event, event->data<TradingDay>().timestamp);
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

        publish_state(get_writer(location::PUBLIC), now());
    }
}

