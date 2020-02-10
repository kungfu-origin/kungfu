//
// Created by Keren Dong on 2020/2/9.
//

#include <utility>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/practice/watcher.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::practice;

namespace kungfu::yijinjing::practice
{
    watcher::watcher(location_ptr &location, bool low_latency) : apprentice(location, low_latency)
    {}

    void watcher::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location)
    {
        if (not has_location(app_location->uid))
        {
            // bypass location events from others master cmd journal
            apprentice::register_location(trigger_time, app_location);
            watch(trigger_time, app_location);
        }
    }

    void watcher::deregister_location(int64_t trigger_time, uint32_t location_uid)
    {
        if (has_location(location_uid))
        {
            apprentice::deregister_location(trigger_time, location_uid);
        }
    }

    void watcher::on_write_to(const event_ptr &event)
    {
        if (event->source() == get_master_commands_uid())
        {
            apprentice::on_write_to(event);
        }
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
