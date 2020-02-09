//
// Created by Keren Dong on 2020/2/9.
//

#include <utility>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/practice/watcher.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::yijinjing::practice
{
    watcher::watcher(location_ptr &location, bool low_latency) : apprentice(location, low_latency)
    {
    }

    void watcher::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
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
                break;
            }
            case category::TD:
            {
                watch(trigger_time, app_location);
                request_write_to(trigger_time, app_location->uid);
                request_read_from(trigger_time, app_location->uid);
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

    void watcher::deregister_location(int64_t trigger_time, uint32_t location_uid)
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

    void watcher::on_write_to(const event_ptr &event)
    {
        if (event->source() == get_master_commands_uid())
        {
            apprentice::on_write_to(event);
        }
    }

    void watcher::on_read_from(const event_ptr &event)
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

    void watcher::on_start()
    {
        apprentice::on_start();
    }

    void watcher::watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
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
}
