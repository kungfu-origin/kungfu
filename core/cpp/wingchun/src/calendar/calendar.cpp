//
// Created by qlu on 2019/4/15.
//

#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace calendar_util
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger)
        {
            spdlog::set_default_logger(logger->clone("calendar"));
        }
    }

    Calendar::Calendar(kungfu::yijinjing::io_device_client_ptr io_device) : io_device_(io_device)
    {}

    Calendar::~Calendar()
    {}

    bool Calendar::is_open(int64_t nano, const std::string& exchange_id)
    {
        std::string trading_day = get_trading_day_from_nano(nano);
        trading_day = get_next_trading_day(trading_day.c_str(),0);
        if(trading_evening[exchange_id])
        {
            int64_t evening_start = get_open_time(trading_day, exchange_id, EVENING);
            int64_t evening_end = get_close_time(trading_day, exchange_id, EVENING);
            if(nano>=evening_start&&nano<=evening_end)
            {
                return true;
            }
        }
        int64_t morning_start = get_open_time(trading_day, exchange_id, MORNING);
        int64_t morning_end = get_close_time(trading_day, exchange_id, MORNING);
        int64_t afternoon_start = get_open_time(trading_day, exchange_id, AFTERNOON);
        int64_t afternoon_end = get_close_time(trading_day, exchange_id, AFTERNOON);
        return ((nano>=morning_start&&nano<=morning_end)||(nano>=afternoon_start&&nano<=afternoon_end));
    }

    int64_t Calendar::get_open_time(const std::string& trading_day, const std::string& exchange_id, int slot)
    {
        if(slot == DEFAULT_SLOT)
        {
            slot = trading_evening[exchange_id]?EVENING:MORNING;
        }
        return get_nano_from_trading_day(trading_day, trading_times[exchange_id][0][slot]);
    }

    int64_t Calendar::get_close_time(const std::string& trading_day, const std::string& exchange_id, int slot)
    {
        if(slot == DEFAULT_SLOT)
        {
            slot = AFTERNOON;
        }
        return get_nano_from_trading_day(trading_day, trading_times[exchange_id][1][slot]);
    }

    int64_t Calendar::next_open(int64_t nano, const std::string& exchange_id, int slot)
    {
        std::string trading_day = get_trading_day_from_nano(nano);
        trading_day = get_next_trading_day(trading_day.c_str(),0);
        if(slot == DEFAULT_SLOT||(!trading_evening[exchange_id]&&slot == EVENING))
        {
            slot = MORNING;
        }
        int64_t start = get_open_time(trading_day, exchange_id, slot);
        if(nano<=start)
        {
            return start;
        }
        trading_day = get_next_trading_day(trading_day.c_str(),1);
        return get_open_time(trading_day, exchange_id, slot);
    }

    int64_t Calendar::next_close(int64_t nano, const std::string& exchange_id, int slot)
    {
        std::string trading_day = get_trading_day_from_nano(nano);
        trading_day = get_next_trading_day(trading_day.c_str(),0);
        if(slot == DEFAULT_SLOT||(!trading_evening[exchange_id]&&slot == EVENING))
        {
            slot = AFTERNOON;
        }
        int64_t start = get_close_time(trading_day, exchange_id, slot);
        if(nano<=start)
        {
            return start;
        }
        trading_day = get_next_trading_day(trading_day.c_str(),1);
        return get_close_time(trading_day, exchange_id, slot);
    }

    std::vector<flying::TradingSession> Calendar::get_trading_sessions(int64_t start_nano, int64_t end_nano, const std::string& exchange_id)
    {
        std::vector<flying::TradingSession> result;
        if(end_nano<start_nano)
        {
            return result;
        }
        std::string start_trading_day = get_trading_day_from_nano(start_nano);
        std::string end_trading_day = get_trading_day_from_nano(end_nano);
        int64_t start,end;
        while(true)
        {
            if(trading_evening[exchange_id])
            {
                int64_t evening_start = get_open_time(start_trading_day, exchange_id, EVENING);
                int64_t evening_end = get_close_time(start_trading_day, exchange_id, EVENING);
                start = evening_start<start_nano?start_nano:evening_start;
                end = evening_end<end_nano?evening_end:end_nano;
                if(start<=end)
                {
                    result.push_back({exchange_id, start_trading_day, start, end});
                }
            }
            int64_t morning_start = get_open_time(start_trading_day, exchange_id, MORNING);
            int64_t morning_end = get_close_time(start_trading_day, exchange_id, MORNING);
            start = morning_start<start_nano?start_nano:morning_start;
            end = morning_end<end_nano?morning_end:end_nano;
            if(start<=end)
            {
                result.push_back({exchange_id, start_trading_day, start, end});
            }
            int64_t afternoon_start = get_open_time(start_trading_day, exchange_id, AFTERNOON);
            int64_t afternoon_end = get_close_time(start_trading_day, exchange_id, AFTERNOON);
            start = afternoon_start<start_nano?start_nano:afternoon_start;
            end = afternoon_end<end_nano?afternoon_end:end_nano;
            if(start<=end)
            {
                result.push_back({exchange_id, start_trading_day, start, end});
            }
            if(start_trading_day==end_trading_day)break;
            start_trading_day = get_next_trading_day(start_trading_day.c_str(), 1);
        }
        return result;
    }

    std::string Calendar::get_current_trading_day()
    {
        if (current_trading_day_.empty())
        {
            get_current_via_req();
        }
        return current_trading_day_;
    }

    std::string Calendar::get_next_trading_day()
    {
        return get_next_trading_day(get_current_trading_day().c_str(), 1);
    }

    std::string Calendar::get_next_trading_day(const char* start_date, int delta) const
    {
        nlohmann::json j;
        j["request"] = "calendar/calculate";
        j["region"] = REGION_CN;
        j["start_date"] = start_date;
        j["delta"] = delta;

        std::string response = io_device_->get_request_socket()->request(j.dump());
        j = nlohmann::json::parse(response);
        return j["data"]["trading_day"];
    }


    void Calendar::get_current_via_req()
    {
        nlohmann::json j;
        j["request"] = "calendar/current";
        j["region"] = REGION_CN;

        std::string response = io_device_->get_request_socket()->request(j.dump());
        j = nlohmann::json::parse(response);
        current_trading_day_ = j["data"]["trading_day"];
    }
}
