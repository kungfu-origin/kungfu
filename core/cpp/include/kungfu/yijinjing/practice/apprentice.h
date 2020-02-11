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
#include <kungfu/yijinjing/practice/hero.h>

namespace kungfu::yijinjing::practice
{
    class apprentice : public hero
    {
    public:
        explicit apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

        void request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time);

        void request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time);

        void request_write_to(int64_t trigger_time, uint32_t dest_id);

        uint32_t get_master_commands_uid()
        {
            return master_commands_location_->uid;
        }

        std::string get_config_db_file(const std::string &name)
        {
            return config_location_->locator->layout_file(config_location_, yijinjing::data::layout::SQLITE, name);
        }

        yijinjing::data::location_ptr get_config_location() const
        { return config_location_; }

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

        template<typename T>
        inline void write_to(int64_t trigger_time, int32_t msg_type, T &data, uint32_t dest_id = 0)
        {
            writers_[dest_id]->write(trigger_time, msg_type, data);
        }

        virtual void on_trading_day(const event_ptr &event, int64_t daytime)
        {}

        void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback);

        void add_time_interval(int64_t nanotime, const std::function<void(event_ptr)> &callback);

    protected:
        longfist::StateMapType state_map_;
        yijinjing::data::location_ptr config_location_;

        void react() override;

        virtual void on_start()
        {}

        virtual void on_read_from(const event_ptr &event);

        virtual void on_read_from_public(const event_ptr &event);

        virtual void on_write_to(const event_ptr &event);

        std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> timer(int64_t nanotime)
        {
            auto writer = writers_[master_commands_location_->uid];
            int32_t timer_usage_count = timer_usage_count_;
            int64_t duration_ns = nanotime - now();
            longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
            r.id = timer_usage_count;
            r.duration = duration_ns;
            r.repeat = 1;
            writer->close_data();
            timer_checkpoints_[timer_usage_count] = now();
            timer_usage_count_++;
            return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src)
            {
                return events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e)
                                            {
                                                return (e->msg_type() == longfist::types::Time::tag &&
                                                        e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns);
                                            }) | rx::first();
            };
        }

        template<typename Duration, typename Enabled = rx::is_duration<Duration>>
        std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> time_interval(Duration &&d)
        {
            auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
            auto writer = writers_[master_commands_location_->uid];
            int32_t timer_usage_count = timer_usage_count_;
            longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
            r.id = timer_usage_count;
            r.duration = duration_ns;
            r.repeat = 1;
            writer->close_data();
            timer_checkpoints_[timer_usage_count] = now();
            timer_usage_count_++;
            return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src)
            {
                return events_ |
                       rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e)
                                  {
                                      if (e->msg_type() == longfist::types::Time::tag &&
                                          e->gen_time() > timer_checkpoints_[timer_usage_count] + duration_ns)
                                      {
                                          auto writer = writers_[master_commands_location_->uid];
                                          longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
                                          r.id = timer_usage_count;
                                          r.duration = duration_ns;
                                          r.repeat = 1;
                                          writer->close_data();
                                          timer_checkpoints_[timer_usage_count] = now();
                                          return true;
                                      } else
                                      {
                                          return false;
                                      }
                                  });
            };
        }

        template<typename Duration, typename Enabled = rx::is_duration<Duration>>
        std::function<rx::observable<event_ptr>(rx::observable<event_ptr>)> timeout(Duration &&d)
        {
            auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
            auto writer = writers_[master_commands_location_->uid];
            int32_t timer_usage_count = timer_usage_count_;
            longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
            r.id = timer_usage_count;
            r.duration = duration_ns;
            r.repeat = 1;
            writer->close_data();
            timer_checkpoints_[timer_usage_count] = now();
            timer_usage_count_++;
            return [&, duration_ns, timer_usage_count](rx::observable<event_ptr> src)
            {
                return (src | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e)
                                         {
                                             if (e->msg_type() != longfist::types::Time::tag)
                                             {
                                                 auto writer = writers_[master_commands_location_->uid];
                                                 longfist::types::TimeRequest &r = writer->open_data<longfist::types::TimeRequest>(0);
                                                 r.id = timer_usage_count;
                                                 r.duration = duration_ns;
                                                 r.repeat = 1;
                                                 writer->close_data();
                                                 timer_checkpoints_[timer_usage_count] = now();
                                                 return true;
                                             } else
                                             {
                                                 return false;
                                             }
                                         })).merge(events_ | rx::filter([&, duration_ns, timer_usage_count](const event_ptr &e)
                                                                        {
                                                                            if (e->gen_time() >
                                                                                timer_checkpoints_[timer_usage_count] + duration_ns)
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
        int32_t timer_usage_count_;
        longfist::recover recover_;

        void checkin();

        void register_location_from_event(const event_ptr &event);

        void deregister_location_from_event(const event_ptr &event);

        template <typename R>
        inline void do_read_from(const event_ptr &event, uint32_t dest_id)
        {
            const R &request = event->data<R>();
            SPDLOG_INFO("{} [{:08x}] requires {} [{:08x}] read from {} [{:08x}] from {}, {}",
                        get_location(event->source())->uname, event->source(),
                        event->dest() == 0 ? "public" : get_location(event->dest())->uname, event->dest(),
                        get_location(request.source_id)->uname, request.source_id,
                        time::strftime(request.from_time),
                        typeid(R).name());
            reader_->join(get_location(request.source_id), dest_id, request.from_time);
        }
    };

    DECLARE_PTR(apprentice)
}

#endif //KUNGFU_APPRENTICE_H
