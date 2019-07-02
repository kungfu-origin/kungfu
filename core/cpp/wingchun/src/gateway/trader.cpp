#include <utility>

//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/trader.h>
#include <kungfu/wingchun/util/business_helper.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

typedef rxcpp::schedulers::scheduler::clock_type::time_point::duration duration_type;

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id)
                    :
                    apprentice(location::make(mode::LIVE, category::TD, source, account_id, std::move(locator)), low_latency),
                    source_(source), account_id_(account_id),
                    calendar_(get_config_db_file("holidays"))
            {
                log::copy_log_settings(get_io_device()->get_home(), account_id);
            }

            void Trader::react(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::react(events);

                events | is(msg::type::OrderInput) |
                $([&](event_ptr event)
                  {
                      insert_order(event);
                  });

                events | is(msg::type::OrderAction) |
                $([&](event_ptr event)
                  {
                      cancel_order(event);
                  });

//                events | time_interval(std::chrono::minutes(1), observe_on_new_thread()) |
//                $([&](duration_type v)
//                  {
//                      auto account_info = account_manager_.get_account_info();
//                      bool is_open = calendar_.is_open(time::now_in_nano(), EXCHANGE_SSE);
//                      if (is_open)
//                      {
//                          account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
//
//                          storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true).insert(
//                                  account_info);
//                      }
//                  });
//
//                events | time_interval(std::chrono::hours(24), observe_on_new_thread()) |
//                $([&](duration_type v)
//                  {
//                      auto account_info = account_manager_.get_account_info();
//                      account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
//                      storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true).insert(account_info);
//                  });
            }

            void Trader::on_start(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::on_start(events);
                auto home = get_io_device()->get_home();
                observe(time::now_in_nano(), location::make(home->mode, category::MD, source_, source_, home->locator));
            }
        }
    }
}
