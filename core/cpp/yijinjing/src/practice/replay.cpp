#include <utility>

//
// Created by Keren Dong on 2019-07-06.
//

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>

#include <kungfu/practice/replay.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;

namespace kungfu
{
    namespace practice
    {
        replay::replay(const location_ptr &home, bool low_latency) : apprentice(home, low_latency)
        {}

        replay::replay(apprentice_ptr app) : replay(app->get_io_device()->get_home(), app->get_io_device()->is_low_latency())
        {
            app_ = std::move(app);
        }

        void replay::react(const observable<event_ptr> &events)
        {
            events |
            $([&](event_ptr e)
              {
                  SPDLOG_INFO("got event {}", e->gen_time());
              });
        }
    }
}
