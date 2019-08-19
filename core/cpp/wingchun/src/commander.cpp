//
// Created by Keren Dong on 2019-08-10.
//

#include <utility>
#include <sstream>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/commander.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        Commander::Commander(locator_ptr locator, bool low_latency, const std::string &name) :
                apprentice(location::make(mode::LIVE, category::SYSTEM, "ledger", name, std::move(locator)), low_latency)
        {
            log::copy_log_settings(get_io_device()->get_home(), name);
        }

        void Commander::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
        {
            if (has_location(app_location->uid))
            {
                // bypass location events from others master cmd journal
                return;
            }
            apprentice::register_location(trigger_time, app_location);
            switch (app_location->category)
            {
                case category::TD:
                {
                    request_write_to(trigger_time, app_location->uid);
                }
                case category::STRATEGY:
                {
                    connect(trigger_time, app_location);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void Commander::deregister_location(int64_t trigger_time, uint32_t location_uid)
        {
            auto app_location = get_location(location_uid);
            apprentice::deregister_location(trigger_time, location_uid);
        }

        void Commander::on_write_to(const yijinjing::event_ptr &event)
        {
            if (event->source() == get_master_commands_uid())
            {
                apprentice::on_write_to(event);
            }
        }

        void Commander::on_read_from(const yijinjing::event_ptr &event)
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

        void Commander::on_start()
        {
            apprentice::on_start();

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
                      const nlohmann::json& cmd = event->data<nlohmann::json>();
                      SPDLOG_INFO("handle command {}", cmd.dump());
                      std::string response = handle_request(event, event->to_string());
                      get_io_device()->get_rep_sock()->send(response);
                  }
                  catch (const std::exception &e)
                  {
                      SPDLOG_ERROR("Unexpected exception {}", e.what());
                  }
              });
        }

        void Commander::connect(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
        {
            auto app_uid_str = fmt::format("{:08x}", app_location->uid);
            auto master_cmd_location = location::make(mode::LIVE, category::SYSTEM, "master", app_uid_str, app_location->locator);
            reader_->join(master_cmd_location, app_location->uid, trigger_time);
            reader_->join(app_location, 0, trigger_time);
        }

        void Commander::new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input)
        {}

        void Commander::cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id)
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