//
// Created by Keren Dong on 2019-06-28.
//

#include <utility>
#include <sstream>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/watcher.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        Watcher::Watcher(locator_ptr locator, mode m, bool low_latency) :
                apprentice(location::make(m, category::SYSTEM, "watcher", "watcher", std::move(locator)), low_latency)
        {
            log::copy_log_settings(get_io_device()->get_home(), "watcher");
            if (m == mode::LIVE)
            {
                pub_sock_ = get_io_device()->bind_socket(nanomsg::protocol::PUBLISH);
            }
        }

        void Watcher::publish(const std::string &msg)
        {
            if (get_io_device()->get_home()->mode == mode::LIVE)
            {
                pub_sock_->send(msg);
            }
            SPDLOG_INFO("published {}", msg);
        }

        void Watcher::publish_state(int64_t trigger_time, yijinjing::data::category c, const std::string &group, const std::string &name,
                                    GatewayState state)
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

        void Watcher::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
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
                    publish_state(trigger_time, app_location->category, app_location->group, app_location->name, GatewayState::Connected);
                    break;
                }
                case category::TD:
                {
                    watch(trigger_time, app_location);
                    publish_state(trigger_time, app_location->category, app_location->group, app_location->name, GatewayState::Connected);
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

        void Watcher::deregister_location(int64_t trigger_time, uint32_t location_uid)
        {
            auto app_location = get_location(location_uid);
            switch (app_location->category)
            {
                case category::MD:
                {
                    publish_state(trigger_time, app_location->category, app_location->group, app_location->name, GatewayState::DisConnected);
                    break;
                }
                case category::TD:
                {
                    publish_state(trigger_time, app_location->category, app_location->group, app_location->name, GatewayState::DisConnected);
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

        void Watcher::on_start()
        {
            apprentice::on_start();

            pre_start();

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
                  std::string response = handle_request(event->to_string());
                  get_io_device()->get_rep_sock()->send(response);
              });

            /**
             * process trade events
             */
            events_ | is(msg::type::Quote) |
            $([&](event_ptr event)
              {
                  on_quote(event, event->data<Quote>());
              });

            events_ | is(msg::type::Order) |
            $([&](event_ptr event)
              {
                  SPDLOG_INFO("hanlde order from {} {:08x}", get_location(event->source())->uname, get_location(event->source())->uid);
                  on_order(event, event->data<Order>());
              });

            events_ | is(msg::type::Trade) |
            $([&](event_ptr event)
              {
                  on_trade(event, event->data<Trade>());
              });

            events_ | is(msg::type::AssetInfo) |
            $([&](event_ptr event)
              {
                  memcpy(&asset_info_, &event->data<AssetInfo>(), sizeof(AssetInfo));
              });

            events_ | is(msg::type::Position) |
            $([&](event_ptr event)
              {
                  position_buffer_.push_back(event->data<Position>());
              });

            events_ | is(msg::type::PositionEnd) |
            $([&](event_ptr event)
              {
                  on_assets(asset_info_, position_buffer_);
                  position_buffer_.clear();
                  memset(&asset_info_, 0, sizeof(asset_info_));
              });
        }

        void Watcher::watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
        {
            auto app_uid_str = fmt::format("{:08x}", app_location->uid);
            auto master_cmd_location = location::make(mode::LIVE, category::SYSTEM, "master", app_uid_str, app_location->locator);
            reader_->join(master_cmd_location, app_location->uid, trigger_time);
            reader_->join(app_location, 0, trigger_time);
        }
    }
}

