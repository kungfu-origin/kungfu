//
// Created by PolarAir on 2019-02-28.
//

#ifndef KUNGFU_CALENDAR_SERVICE_H
#define KUNGFU_CALENDAR_SERVICE_H

#include <atomic>
#include <thread>
#include <vector>

#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    class CalendarService
    {
    public:
        CalendarService();
        virtual ~CalendarService();
        void run();
        void join();
        void stop();
        int get_current_day() const;
        int calculate_trading_day(int date, int delta) const;

    protected:
        void loop_new_date();
        void loop_reqs();
        void publish_via_nanomsg(int socket, const std::string& date, MsgType msg_type);

    protected:
        int rsp_socket_;
        int pub_socket_;
        std::vector<std::shared_ptr<std::thread>> threads_;
        static std::atomic<bool> started_;
        std::atomic<int> current_;
        std::vector<int> trading_days_;
    private:
        static void signal_handler(int signal);
    };
} // namespace kungfu

#endif // KUNGFU_CALENDAR_SERVICE_H
