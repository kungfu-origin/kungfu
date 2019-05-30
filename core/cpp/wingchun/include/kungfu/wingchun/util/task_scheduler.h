//
// Created by PolarAir on 2019-03-11.
//

#ifndef KUNGFU_TASK_SCHEDULER_H
#define KUNGFU_TASK_SCHEDULER_H

#include <functional>
#include <thread>
#include <atomic>
#include <map>

namespace kungfu
{
    /*
     * 时间触发的任务调度器，nano time 由外部输入，可以注册并执行定时任务。
     *
     * insert_callback_xx函数用于注册回调
     * 如果async = false，则在update_nano返回前回调
     * 如果async = true, 则在新线程中回调
     *
     * 触发器依赖upate_nano来更新时间，即所有回调都将由此函数触发
     *
     * insert_callback_at_next中time_str格式为"09:01:07"
     */
    typedef std::function<void (int64_t)> TSCallback;
    class TaskScheduler
    {
    public:
        TaskScheduler();
        virtual ~TaskScheduler();
        void run();
        void stop();

        void insert_callback_at(int64_t nano, TSCallback cb, bool async = false);
        void insert_callback_after(int msec, TSCallback cb, bool async = false);
        void insert_callback_at_next(const char* time_str, TSCallback cb, bool async= false);

        void update_nano(int64_t nano);
        int64_t get_nano() const { return nano_; }
    protected:
        struct TSCallbackUnit
        {
            int         id_;
            int64_t        nano_;
            TSCallback  cb_;
            bool        async_;
        };

    protected:
        int get_id() const;
        void run_async_unit(const TSCallbackUnit& unit);
        void remove_async_unit(int64_t nano, int id);

    protected:
        std::map<int, std::shared_ptr<std::thread>>     threads_;
        std::atomic<bool>                               started_;
        std::atomic<int64_t>                               nano_;
        std::map<int, TSCallbackUnit>                   cbs_;
        // TODO: consider priority_queue
    };
}

#endif //KUNGFU_TASK_SCHEDULER_H
