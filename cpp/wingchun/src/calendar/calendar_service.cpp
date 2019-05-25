//
// Created by PolarAir on 2019-02-28.
//

#include <ctime>
#include <fmt/format.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <signal.h>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>

#include <kungfu/wingchun/calendar/calendar_service.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/constant.h>
#include "calendar_storage.hpp"

/*
 *  file: CALENDAR_HOLIDAY_DB_FILE (config.h)
 *  table: holidays
 *  columns: region, holiday
 *
 *  region取值见constant.h REGION_XX，目前只有cn/hk，并且接口并不暴露这个参数，内部存储先做支持
 */

namespace kungfu
{
    std::atomic<bool> CalendarService::started_(false);

    CalendarService::CalendarService(): rsp_socket_(-1), pub_socket_(-1), current_(0)
    {
        kungfu::log::LogConfig::setup_log("calendar");
        SPDLOG_INFO("CalendarService initializing with base dir: {}", get_base_dir());
        std::string calendar_db_file = fmt::format(CALENDAR_HOLIDAY_DB_FILE_FORMAT, get_base_dir());
        SPDLOG_INFO("CalendarService storage at {}", calendar_db_file);
        CalendarStorage storage(calendar_db_file);
        SPDLOG_INFO("CalendarService storage");
        storage.get_trading_days(1990, 2090, trading_days_, REGION_CN);
        SPDLOG_INFO("CalendarService initialized");
    }

    CalendarService::~CalendarService()
    {
        stop();
    }

    void CalendarService::signal_handler(int signal)
    {
        SPDLOG_TRACE("signal received {}", signal);
        started_ = false;
    }

    void CalendarService::run()
    {
        for (int s = 1; s < 32; s++)
        {
            signal(s, CalendarService::signal_handler);
        }
        if (started_)
        {
            return;
        }

        std::string rep_url = fmt::format(CALENDAR_REP_URL, get_base_dir());
        rsp_socket_ = nn_socket(AF_SP, NN_REP);
        int timeout = 1000;
        nn_setsockopt(rsp_socket_, NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(int));
        if (auto ret = nn_bind(rsp_socket_, rep_url.c_str()) < 0)
        {
            SPDLOG_ERROR("[CalendarService] exception: rep socket create failed, ret = {}", ret);
            return;
        }

        std::string pub_url = fmt::format(CALENDAR_PUB_URL, get_base_dir());
        pub_socket_ = nn_socket(AF_SP, NN_PUB);
        if (auto ret = nn_bind(pub_socket_, pub_url.c_str()) < 0)
        {
            SPDLOG_ERROR("[CalendarService] exception: pub socket create failed, ret = {}", ret);
            return;
        }

        SPDLOG_INFO("[CalendarService] started with pub: {}, rep: {}", pub_url, rep_url);
        started_ = true;
        threads_.emplace_back(std::shared_ptr<std::thread>(new std::thread(&CalendarService::loop_new_date, this)));
        threads_.emplace_back(std::shared_ptr<std::thread>(new std::thread(&CalendarService::loop_reqs, this)));
    }

    void CalendarService::join()
    {
        if (!started_)
        {
            return;
        }

        for (const auto& thread : threads_)
        {
            thread->join();
        }
    }

    void CalendarService::stop()
    {
        if (!started_)
        {
            return;
        }

        started_ = false;
        for (const auto& thread : threads_)
        {
            thread->join();
        }
    }

    void CalendarService::loop_new_date()
    {
        while (started_)
        {
            int new_current = get_current_day();
            if (new_current != current_)
            {
                if (current_ != 0)
                {
                    SPDLOG_INFO("[CalendarService] trading day changed from {} to {}", current_, new_current);
                    publish_via_nanomsg(pub_socket_, std::to_string(new_current), MsgType::SwitchDay);
                }
                current_ = new_current;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        nn_shutdown(pub_socket_, 0);
    }

    void CalendarService::loop_reqs()
    {
        while (started_)
        {
            // process req msgs
            char* buf = nullptr;
            int bytes = nn_recv(rsp_socket_, &buf, NN_MSG, 0);
            if (bytes > 0)
            {
                try
                {
                    std::string recv(buf);
                    SPDLOG_INFO("[CalendarService] receiving request: {}", recv);
                    nlohmann::json recv_j = nlohmann::json::parse(recv);
                    std::string req = recv_j["req"];

                    if (req == "current")
                    {
                        publish_via_nanomsg(rsp_socket_, std::to_string(current_), MsgType::RspTradingDay);
                    }
                    else if (req == "calc")
                    {
                        int day = std::stoi(recv_j["start_date"].get<std::string>());
                        int delta = recv_j["delta"].get<int>();
                        publish_via_nanomsg(rsp_socket_, std::to_string(calculate_trading_day(day, delta)), MsgType::RspTradingDay);
                    }
                    else
                    {
                        SPDLOG_WARN("invalid request {}", req);
                    }
                    
                }
                catch (...)
                {
                    SPDLOG_ERROR("[CalendarService] exception: parse request failed");
                }

                nn_freemsg(buf);
            }
        }
        nn_shutdown(rsp_socket_, 0);
    }

    int CalendarService::get_current_day() const
    {
        auto cur_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int cur_date = std::stoi(yijinjing::parseNano(cur_nano, "%Y%m%d"));
        int cur_hour = std::stoi(yijinjing::parseNano(cur_nano, "%H"));
        return calculate_trading_day(cur_date, cur_hour >= SWITCH_HOUR ? 1 : 0);
    }

    int CalendarService::calculate_trading_day(int date, int delta) const
    {
        int trading_days_size = trading_days_.size();
        int lo = 0, hi = trading_days_size;
        while (lo < hi)
        {
            int mi = (lo + hi) >> 1;
            (date <= trading_days_[mi]) ? hi = mi : lo = mi + 1;
        }
        int found = lo;
        int ret;
        if (found + delta < trading_days_size && found + delta >= 0)
        {
            ret = trading_days_[found + delta];
        }
        else if (found + delta < 0)
        {
            ret = trading_days_[0];
        }
        else
        {
            ret = trading_days_[trading_days_size - 1];
        }
        return ret;
    }

    void CalendarService::publish_via_nanomsg(int socket, const std::string &date, MsgType msg_type)
    {
        nlohmann::json data;
        data["trading_day"] = date;
        nlohmann::json j;
        j["msg_type"] = msg_type;
        j["data"] = data;
        std::string js = j.dump();
        SPDLOG_TRACE("[CalendarService] sending response: {}", js);
        nn_send(socket, js.c_str(), js.size() + 1, 0);
    }

} // namespace kungfu
