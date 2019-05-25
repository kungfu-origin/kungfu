//
// Created by PolarAir on 2019-03-11.
//

#include <kungfu/time/timer.h>
#include <kungfu/wingchun/util/task_scheduler.h>

namespace kungfu
{
    TaskScheduler::TaskScheduler() : started_(false), nano_(0)
    {

    }

    TaskScheduler::~TaskScheduler()
    {
        stop();
    }

    void TaskScheduler::run()
    {
        if (started_)
        {
            return;
        }

        started_ = true;
    }

    void TaskScheduler::stop()
    {
        if (!started_)
        {
            return;
        }

        started_ = false;
    }

    void TaskScheduler::insert_callback_at(int64_t nano, kungfu::TSCallback cb, bool async)
    {
        int id = get_id();
        TSCallbackUnit unit{id, nano, cb, async};
        cbs_[id] = unit;
        if (nano <= nano_)
        {
            update_nano(nano_); // trigger
        }
    }

    void TaskScheduler::insert_callback_after(int msec, kungfu::TSCallback cb, bool async)
    {
        insert_callback_at(nano_ + msec * 1000000, cb, async);
    }

    void TaskScheduler::insert_callback_at_next(const char *time_str, kungfu::TSCallback cb, bool async)
    {
        string cur_time_str = yijinjing::parseNano(nano_, "%Y%m%d-");
        string date_time_str = cur_time_str + time_str;
        int64_t nano = yijinjing::parseTime(date_time_str.c_str(), "%Y%m%d-%H:%M:%S");
        if (nano <= nano_)
        {
            nano += yijinjing::NANOSECONDS_PER_DAY;
        }
        insert_callback_at(nano, cb, async);
    }

    void TaskScheduler::update_nano(int64_t nano)
    {
        if (nano <= nano_)
        {
            return;
        }
        nano_ = nano;
        for (auto it = cbs_.begin(); it != cbs_.end();)
        {
            if (it->second.nano_ <= nano)
            {
                if (it->second.async_)
                {
                    auto thread = std::make_shared<std::thread>(&TaskScheduler::run_async_unit, this, it->second);
                    threads_[it->second.id_] = thread;
                }
                else
                {
                    it->second.cb_(nano_);
                    it = cbs_.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    int TaskScheduler::get_id() const
    {
        static int id = 0;
        return ++id;
    }

    void TaskScheduler::run_async_unit(const TSCallbackUnit &unit)
    {
        unit.cb_(nano_);

        if (started_)
        {
            insert_callback_at(0, std::bind(&TaskScheduler::remove_async_unit, this, std::placeholders::_1, unit.id_));
        }
    }

    void TaskScheduler::remove_async_unit(int64_t nano, int id)
    {
        cbs_.erase(id);
    }
}