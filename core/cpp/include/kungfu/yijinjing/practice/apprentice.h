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

        bool is_started() const;

        uint32_t get_master_commands_uid() const;

        int64_t get_master_start_time() const;

        yijinjing::data::location_ptr get_config_location() const;

        void request_read_from(int64_t trigger_time, uint32_t source_id, int64_t from_time);

        void request_read_from_public(int64_t trigger_time, uint32_t source_id, int64_t from_time);

        void request_write_to(int64_t trigger_time, uint32_t dest_id);

        void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback);

        void add_time_interval(int64_t nanotime, const std::function<void(event_ptr)> &callback);

        virtual void on_trading_day(const event_ptr &event, int64_t daytime);

        template<typename DataType>
        inline void write_to(int64_t trigger_time, DataType &data, uint32_t dest_id = 0)
        {
            writers_[dest_id]->write(trigger_time, data);
        }

    protected:
        longfist::StateMapType state_map_;
        yijinjing::data::location_ptr config_location_;

        void react() override;

        virtual void on_start();

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
        int64_t master_start_time_;
        std::unordered_map<int, int64_t> timer_checkpoints_;
        int32_t timer_usage_count_;
        longfist::recover recover_state;
        bool started_;

        void checkin();

        template <typename DataType>
        void register_location_from_event(const event_ptr &event)
        {
            auto app_location = data::location::make_shared(event->data<DataType>(), get_io_device()->get_home()->locator);
            register_location(event->trigger_time(), app_location);
        }

        template <typename DataType>
        void deregister_location_from_event(const event_ptr &event)
        {
            uint32_t location_uid = data::location::make_shared(event->data<DataType>(), get_io_device()->get_home()->locator)->uid;
            reader_->disjoin(location_uid);
            deregister_channel_by_source(location_uid);
            deregister_location(event->trigger_time(), location_uid);
        }

        template <typename DataType>
        inline void do_read_from(const event_ptr &event, uint32_t dest_id)
        {
            const DataType &request = event->data<DataType>();
            SPDLOG_INFO("{} requires {} read from {} from {}",
                        get_location(event->source())->uname,
                        get_location(event->dest())->uname,
                        get_location(request.source_id)->uname,
                        time::strftime(request.from_time));
            reader_->join(get_location(request.source_id), dest_id, request.from_time);
        }
    };

    DECLARE_PTR(apprentice)
}

#endif //KUNGFU_APPRENTICE_H
