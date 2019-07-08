/*****************************************************************************
 * Copyright [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by Keren Dong on 2019-06-01.
//

#ifndef KUNGFU_APPRENTICE_H
#define KUNGFU_APPRENTICE_H

#include <unordered_map>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/hero.h>

namespace kungfu
{
    namespace practice
    {
        class apprentice : public hero
        {
        public:
            apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

            void request_write_to(int64_t trigger_time, uint32_t dest_id);

            void request_read_from(int64_t trigger_time, uint32_t source_id, bool pub = false);

            uint32_t get_master_commands_uid()
            {
                return master_commands_location_->uid;
            }

            std::string get_config_db_file(const std::string &name)
            {
                return config_location_->locator->layout_file(config_location_, yijinjing::data::layout::SQLITE, name);
            }

            std::string get_app_db_file(const std::string &name, bool default_to_system = false)
            {
                auto home = get_io_device()->get_home();
                if (default_to_system)
                {
                    return home->locator->default_to_system_db(home, name);
                } else
                {
                    return home->locator->layout_file(home, yijinjing::data::layout::SQLITE, name);
                }
            }

        protected:
            yijinjing::data::location_ptr config_location_;

            void react(const rx::observable<yijinjing::event_ptr> &events) override;

            virtual void on_start(const rx::observable<yijinjing::event_ptr> &events)
            {}

            virtual void on_write_to(const yijinjing::event_ptr &event);

            virtual void on_read_from(const yijinjing::event_ptr &event);

            template<typename Duration,
                    typename Enabled = rx::enable_if_all_true_type_t<rx::is_duration<Duration>>>
            std::function<rx::observable<yijinjing::event_ptr>(rx::observable<yijinjing::event_ptr>)> timeout(Duration &&d)
            {
                SPDLOG_INFO("timeout request");
                auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
                auto writer = writers_[master_commands_location_->uid];
                msg::data::TimeRequest &request = writer->open_data<msg::data::TimeRequest>(0, msg::type::TimeRequest);
                request.duration = duration_ns;
                request.repeat = 1;
                int timer_usage_count = timer_usage_count_;
                timer_checkpoints_[timer_usage_count] = now_;
                timer_usage_count_++;
                return [&, duration_ns, timer_usage_count](rx::observable<yijinjing::event_ptr> src)
                {
                    return (src | rx::filter([&, duration_ns, timer_usage_count](yijinjing::event_ptr e)
                                         {
                                             auto writer = writers_[master_commands_location_->uid];
                                             msg::data::TimeRequest &request = writer->open_data<msg::data::TimeRequest>(0, msg::type::TimeRequest);
                                             request.duration = duration_ns;
                                             request.repeat = 1;
                                             timer_checkpoints_[timer_usage_count] = now_;
                                             return true;
                                         })).merge(events_ | rx::filter([&, duration_ns](yijinjing::event_ptr e)
                                                                    {
                                                                        if (e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns)
                                                                        {
                                                                            throw rx::timeout_error("timeout");
                                                                        }
                                                                        return false;
                                                                    }));
                };
            }

        private:
            yijinjing::data::location_ptr master_home_location_;
            yijinjing::data::location_ptr master_commands_location_;
            std::unordered_map<int, int64_t> timer_checkpoints_;
            int timer_usage_count_;

            void checkin();

            void register_location_from_event(const yijinjing::event_ptr &event);
        };

        DECLARE_PTR(apprentice)
    }
}

#endif //KUNGFU_APPRENTICE_H
