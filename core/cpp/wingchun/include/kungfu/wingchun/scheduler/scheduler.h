//
// Created by qlu on 2019/6/17.
//

#ifndef KUNGFU_SCHEDULER_H
#define KUNGFU_SCHEDULER_H

#include "cron.h"
#include <iomanip>
#include <map>
#include <iterator>
#include <kungfu/yijinjing/time.h>

namespace kungfu
{
    namespace
    {
        class Task
        {
        public:
            Task(std::function<void()> &&f, bool recur = false) :
                    f_(std::move(f)), recur_(recur)
            {}

            bool is_recurred() const
            { return recur_; }

            virtual void exec(int64_t trigger_time)
            { f_(); }

            virtual int64_t get_new_time() const = 0;

        private:
            std::function<void()> f_;
            bool recur_;
        };

        class InTask : public Task
        {
        public:
            InTask(std::function<void()> &&f) : Task(std::move(f))
            {}

            int64_t get_new_time() const override
            { return 0; }
        };

        class EveryTask : public Task
        {
        public:
            EveryTask(int64_t start_time, int64_t duration, std::function<void()> &&f) :
                    Task(std::move(f), true), trigger_time_(start_time), duration_(duration)
            {}

            int64_t get_new_time() const override
            {
                return trigger_time_ + duration_;
            };

            void exec(int64_t trigger_time) override
            {
                Task::exec(trigger_time);
                trigger_time_ = trigger_time;
            }

        private:
            int64_t trigger_time_;
            int64_t duration_;
        };

        class Scheduler;

        class CronTask : public Task
        {
        public:
            CronTask(const Scheduler &scheduler, const std::string &expression, std::function<void()> &&f) :
                    scheduler_(scheduler), Task(std::move(f), true), cron_(expression)
            {}

            int64_t get_new_time() const override;

        private:
            const Scheduler &scheduler_;
            Cron cron_;
        };

        inline bool try_parse(std::tm &tm, const std::string &expression, const std::string &format)
        {
            std::stringstream ss(expression);
            return !(ss >> std::get_time(&tm, format.c_str())).fail();
        }

        class Scheduler
        {
        public:
            Scheduler(int64_t nano = kungfu::yijinjing::time::now_in_nano())
            {
                update_time(nano);
            }

            Scheduler(const Scheduler &) = delete;

            Scheduler(Scheduler &&) noexcept = delete;

            Scheduler &operator=(const Scheduler &) = delete;

            Scheduler &operator=(Scheduler &&) noexcept = delete;

            void update_time(int64_t nano)
            {
                if (nano <= nano_)
                {
                    return;
                } else
                {
                    nano_ = nano;
                    manage_tasks();
                }
            }

            template<typename _Callable, typename... _Args>
            void in(int64_t time, _Callable &&f, _Args &&... args)
            {
                std::shared_ptr<Task> t = std::make_shared<InTask>(std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
                add_task(time, std::move(t));
            };

            template<typename _Callable, typename... _Args>
            void in(const std::chrono::system_clock::time_point time, _Callable &&f, _Args &&... args)
            {
                std::shared_ptr<Task> t = std::make_shared<InTask>(std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
                auto nanoseconds_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count();
                add_task(nanoseconds_since_epoch, std::move(t));
            }

            template<typename _Callable, typename... _Args>
            void in(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
            {
                in(now_in_nano() + std::chrono::duration_cast<std::chrono::nanoseconds>(time).count(), std::forward<_Callable>(f),
                   std::forward<_Args>(args)...);
            }

            template<typename _Callable, typename... _Args>
            void at(const std::string &time, _Callable &&f, _Args &&... args)
            {
                time_t time_now = now_in_seconds();
                std::tm tm = *std::localtime(&time_now);

                std::chrono::system_clock::time_point tp;

                if (try_parse(tm, time, "%H:%M:%S"))
                {
                    tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                    if (std::chrono::system_clock::now() >= tp)
                        tp += std::chrono::hours(24);
                } else if (try_parse(tm, time, "%Y-%m-%d %H:%M:%S"))
                {
                    tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                } else if (try_parse(tm, time, "%Y/%m/%d %H:%M:%S"))
                {
                    tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                } else
                {
                    throw std::runtime_error("Cannot parse time string: " + time);
                }

                in(tp, std::forward<_Callable>(f), std::forward<_Args>(args)...);
            }

            template<typename _Callable, typename... _Args>
            void every(const int64_t duration, _Callable &&f, _Args &&... args)
            {
                std::shared_ptr<Task> t = std::make_shared<EveryTask>(now_in_nano(), duration,
                                                                      std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
                auto next_time = t->get_new_time();
                add_task(next_time, std::move(t));
            }

            template<typename _Callable, typename... _Args>
            void every(const std::chrono::system_clock::duration time, _Callable &&f, _Args &&... args)
            {
                every(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count(), std::forward<_Callable>(f), std::forward<_Args>(args)...);
            }

// expression format:
//    ┌───────────── minute (0 - 59)
//    │ ┌───────────── hour (0 - 23)
//    │ │ ┌───────────── day of month (1 - 31)
//    │ │ │ ┌───────────── month (1 - 12)
//    │ │ │ │ ┌───────────── day of week (0 - 6) (Sunday to Saturday)
//    │ │ │ │ │
//    │ │ │ │ │
//    * * * * *
            template<typename _Callable, typename... _Args>
            void cron(const std::string &expression, _Callable &&f, _Args &&... args)
            {
                std::shared_ptr<Task> t = std::make_shared<CronTask>(*this, expression,
                                                                     std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));
                auto next_time = t->get_new_time();
                add_task(next_time, std::move(t));
            }

            int64_t now_in_nano() const
            {
                return nano_;
            }

            int64_t now_in_seconds() const
            {
                return now_in_nano() / kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND;
            }

        private:
            int64_t nano_;
            std::multimap<int64_t, std::shared_ptr<Task>> tasks_;

            void add_task(int64_t time, std::shared_ptr<Task> t)
            {
                tasks_.emplace(time, std::move(t));
            }

            void manage_tasks()
            {
                auto end_of_tasks_to_run = tasks_.upper_bound(now_in_nano());
                if (end_of_tasks_to_run != tasks_.begin())
                {
                    decltype(tasks_) recurred_tasks;
                    for (auto i = tasks_.begin(); i != end_of_tasks_to_run; ++i)
                    {
                        auto &task = (*i).second;
                        task->exec(now_in_nano());
                        if (task->is_recurred())
                        {
                            recurred_tasks.emplace(task->get_new_time(), std::move(task));
                        }
                    }
                    tasks_.erase(tasks_.begin(), end_of_tasks_to_run);
                    for (auto &task : recurred_tasks)
                    {
                        tasks_.emplace(task.first, std::move(task.second));
                    }
                }
            }
        };

        int64_t CronTask::get_new_time() const
        {
            auto from = std::chrono::system_clock::from_time_t(scheduler_.now_in_seconds());
            auto next = cron_.cron_to_next(from);
            auto nanoseconds_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(next.time_since_epoch()).count();
            return nanoseconds_since_epoch;
        }
    }
}

#endif //KUNGFU_SCHEDULER_H
