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

    Calendar::Calendar() : req_socket_(-1), sub_socket_(-1), started_(false), current_(0)
    {
        std::string req_url = fmt::format(CALENDAR_REP_URL, get_base_dir());
        req_socket_ = nn_socket(AF_SP, NN_REQ);
        nn_connect(req_socket_, req_url.c_str());

        std::string sub_url = fmt::format(CALENDAR_PUB_URL, get_base_dir());
        sub_socket_ = nn_socket(AF_SP, NN_SUB);
        nn_setsockopt(sub_socket_, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
        int timeout = 1000;
        nn_setsockopt(sub_socket_, NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(int));
        nn_connect(sub_socket_, sub_url.c_str());

        started_ = true;
        thread_ = std::make_shared<std::thread>(&Calendar::loop, this);
    }

    Calendar::~Calendar()
    {
        if (started_)
        {
            started_ = false;
            nn_shutdown(req_socket_, 0);
            thread_->join();
        }
    }

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

    std::string Calendar::get_current_trading_day(bool via_net)
    {
        if (via_net || current_ == 0)
        {
            get_current_via_req();
        }
        return std::to_string(current_);
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
        std::string req_msg = j.dump();
        nn_send(req_socket_, req_msg.c_str(), req_msg.size(), 0);
        char* buf = nullptr;
        int bytes = nn_recv(req_socket_, &buf, NN_MSG, 0);
        if (bytes >= 0)
        {
            std::string response(buf, bytes);
            nlohmann::json j = nlohmann::json::parse(response);
            std::string date = j["data"]["trading_day"];
            nn_freemsg(buf);
            return date;
        }
        return "";
    }

    void Calendar::register_switch_day_callback(SwitchDayCallback cb)
    {
        cbs_.push_back(cb);
    }

    void Calendar::loop()
    {
        while (started_)
        {
            // process pub msgs
            char* buf = nullptr;
            std::string date;
            bool has_new = false;
            int bytes = nn_recv(sub_socket_, &buf, NN_MSG, 0);
            if (bytes > 0)
            {
                std::string notice(buf, bytes);
                nlohmann::json j = nlohmann::json::parse(notice);
                if (j.find("msg_type") != j.end())
                {
                    MsgType notice_type = j["msg_type"];
                    if (notice_type == MsgType::SwitchDay)
                    {
                        date = j["data"]["trading_day"];
                        SPDLOG_INFO("Switch day to {}", date);
                        current_ = std::stoi(date);
                        has_new = true;
                    }
                }
                nn_freemsg(buf);
            }

            if (has_new)
            {
                for (auto& cb : cbs_)
                {
                    cb(date);
                }
            }
        }
        nn_shutdown(sub_socket_, 0);
    }

    void Calendar::get_current_via_req()
    {
        nlohmann::json j;
        j["request"] = "calendar/current";
        j["region"] = REGION_CN;
        std::string req_msg = j.dump();
        nn_send(req_socket_, req_msg.c_str(), req_msg.size(), 0);
        char* buf = nullptr;
        int bytes = nn_recv(req_socket_, &buf, NN_MSG, 0);
        if (bytes > 0)
        {
            std::string response(buf, bytes);
            nlohmann::json j = nlohmann::json::parse(response);
            std::string date = j["data"]["trading_day"];
            current_ = std::stoi(date);
            nn_freemsg(buf);
        }
    }
}
