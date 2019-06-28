#include <utility>

//
// Created by Keren Dong on 2019-06-28.
//

#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/watcher.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

namespace kungfu
{
    namespace wingchun
    {
        Watcher::Watcher(bool low_latency, locator_ptr locator) :
                apprentice(location::make(mode::LIVE, category::SYSTEM, "watcher", "watcher", std::move(locator)), low_latency),
                calendar_(get_config_db_file("holidays"))
        {
            log::copy_log_settings(get_io_device()->get_home(), "watcher");
            pub_sock_ = get_io_device()->bind_socket(nanomsg::protocol::PUBLISH);
        }

        void Watcher::publish(const std::string &msg)
        {
            pub_sock_->send(msg);
            SPDLOG_INFO("published {}", msg);
        }

        void Watcher::publish_state(int64_t trigger_time, yijinjing::data::category c, const std::string &group, const std::string &name, GatewayState state)
        {
            nlohmann::json msg;
            msg["gen_time"] = time::now_in_nano();
            msg["trigger_time"] = trigger_time;
            msg["msg_type"] = msg::type::GatewayState;
            msg["source"] = get_io_device()->get_home()->uid;

            nlohmann::json data;
            data["category"] = c;
            data["group"] = group;
            data["name"] = name;
            data["state"] = state;
            msg["data"] = data;
            publish(msg.dump());
        }

        void Watcher::react(const rx::observable<yijinjing::event_ptr> &events)
        {
            apprentice::react(events);

            events | is(yijinjing::msg::type::Register) |
            $([&](event_ptr e)
              {
                  auto app_location = get_location(e->source());
                  if (app_location->uid != get_home_uid())
                  {
                      auto app_uid_str = fmt::format("{:08x}", app_location->uid);
                      auto master_cmd_location = location::make(mode::LIVE, category::SYSTEM, "master", app_uid_str, app_location->locator);
                      reader_->join(master_cmd_location, app_location->uid, e->gen_time());
                  }

                  switch (app_location->category)
                  {
                      case category::MD:
                      {
                          observe(app_location, e->gen_time());
                          publish_state(e->gen_time(), app_location->category, app_location->group, app_location->name, GatewayState::Connected);
                          break;
                      }
                      case category::TD:
                      {
                          publish_state(e->gen_time(), app_location->category, app_location->group, app_location->name, GatewayState::Connected);
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
              });

            events | is(yijinjing::msg::type::Deregister) |
            $([&](event_ptr e)
              {
                  auto app_location = get_location(e->source());
                  switch (app_location->category)
                  {
                      case category::MD:
                      {
                          publish_state(e->gen_time(), app_location->category, app_location->group, app_location->name, GatewayState::DisConnected);
                          break;
                      }
                      case category::TD:
                      {
                          publish_state(e->gen_time(), app_location->category, app_location->group, app_location->name, GatewayState::DisConnected);
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
              });

            /**
             * process active query from clients
             */
            events |
            filter([&](event_ptr event)
                   {
                       return dynamic_cast<nanomsg::nanomsg_json *>(event.get()) != nullptr and event->source() == 0;
                   }) |
            $([&](event_ptr event)
              {
                  // let python do the actual job, we just operate the I/O part
                  std::string response = handle_request(event->to_string());
                  get_io_device()->get_rep_sock()->send(response);
              });

            /**
             * process trade events
             */
            events | is(msg::type::Quote) |
            $([&](event_ptr event)
              {
                  on_quote(event, event->data<Quote>());
              });

            events | is(msg::type::Order) |
            $([&](event_ptr event)
              {
                  on_order(event, event->data<Order>());
              });

            events | is(msg::type::Trade) |
            $([&](event_ptr event)
              {
                  on_trade(event, event->data<Trade>());
              });
        }

        void Watcher::on_write_to(const yijinjing::event_ptr &event)
        {
            if (event->source() == get_master_commands_uid())
            {
                apprentice::on_write_to(event);
            }
        }

        void Watcher::on_read_from(const yijinjing::event_ptr &event)
        {
            const yijinjing::msg::data::RequestReadFrom &request = event->data<yijinjing::msg::data::RequestReadFrom>();
            auto source_location = get_location(request.source_id);
            auto dest_location = get_location(event->source());
            if (source_location->uid == get_master_commands_uid())
            {
                apprentice::on_read_from(event);
                return;
            }
            if (source_location->category == category::TD && dest_location->category == category::STRATEGY)
            {
                reader_->join(source_location, dest_location->uid, event->gen_time());
            }
        }

        void Watcher::start()
        {
            apprentice::start();
        }
    }
}

