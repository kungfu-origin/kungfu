//
// Created by PolarAir on 2019-02-28.
//

#ifndef KUNGFU_CALENDAR_H
#define KUNGFU_CALENDAR_H

#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <map>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/constant.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/util/timer_util.h>

namespace kungfu
{
    namespace calendar_util
    {
        void set_logger(std::shared_ptr<spdlog::logger> logger);
    }

    #define MORNING 0
    #define AFTERNOON 1
    #define EVENING 2
    #define DEFAULT_SLOT -1

    typedef std::function<void (const std::string&)> SwitchDayCallback;

    class Calendar
    {
    public:
        Calendar();
        virtual ~Calendar();

        //判断是否是交易时间
        //@param nano 纳秒时间戳
        //@param exchange_id 交易所ID
        //@return 如果在交易时段返回 true, 否则返回 false
        bool is_open(int64_t nano, const std::string& exchange_id);

        //获取开盘时间
        //@param trading_day 交易日 "%Y%m%d"
        //@param exchange_id 交易所ID
        //@param slot 上午/下午/夜盘
        //@return 交易日开盘时间纳秒时间戳，默认取夜盘开盘，若无夜盘则取上午开盘
        //@remark 必须保证 trading_day 为实际交易日
        int64_t get_open_time(const std::string& trading_day, const std::string& exchange_id, int slot = DEFAULT_SLOT);

        //获取收盘时间
        //@param trading_day 交易日 "%Y%m%d"
        //@param exchange_id 交易所ID
        //@param slot 上午/下午/夜盘
        //@return 交易所收盘时间纳秒时间戳，默认下午收盘
        //@remark 必须保证 trading_day 为实际交易日
        int64_t get_close_time(const std::string& trading_day, const std::string& exchange_id, int slot = DEFAULT_SLOT);

        //获取下一个开盘时间
        //@param nano 纳秒时间戳
        //@param exchange_id 交易所ID
        //@return 交易日下一个开盘时间纳秒时间戳，默认上午开盘
        int64_t next_open(int64_t nano, const std::string& exchange_id, int slot = DEFAULT_SLOT);

        //获取下一个收盘时间
        //@param nano 纳秒时间戳
        //@param exchange_id 交易所ID
        //@return 交易日下一个收盘时间纳秒时间戳，默认下午收盘
        int64_t next_close(int64_t nano, const std::string& exchange_id, int slot = DEFAULT_SLOT);

        //获取时间区间内的 trading session
        //@param start_nano 开始纳秒时间戳
        //@param end_nano 结束纳秒时间戳
        //@param exchange_id 交易所ID
        //@return 交易所交易时段列表
        std::vector<flying::TradingSession> get_trading_sessions(int64_t start_nano, int64_t end_nano, const std::string& exchange_id);

        //获取当前交易日
        //@param via_net 是否通过网络获取，如果为 true，发送请求到 calendar service 获取当前交易日
        //@return 当前交易日
        std::string get_current_trading_day(bool via_net = false);

        //获取下一个交易日
        //@return 下一个交易日
        std::string get_next_trading_day();

        //获取 start_date 之后 delta 个交易日日期
        //@return 交易日
        std::string get_next_trading_day(const char* start_date, int delta) const;

        //注册交易日切换回调函数
        //@param cb 交易日切换回调函数
        void register_switch_day_callback(SwitchDayCallback cb);

    private:
        void loop();
        void get_current_via_req();

    protected:
        std::map<std::string, std::vector<std::vector<std::string>>> trading_times {
            {"CZCE", {{"9:00:00", "13:30:00", "21:00:00"}, {"11:30:00", "15:00:00", "23:30:00"}}},
            {"SHFE", {{"9:00:00", "13:30:00", "21:00:00"}, {"11:30:00", "15:00:00", "2:30:00"}}},
            {"DCE", {{"9:00:00", "13:30:00", "21:00:00"}, {"11:30:00", "15:00:00", "23:30:00"}}},
            {"CFFEX", {{"9:00:00", "13:30:00"}, {"11:30:00", "15:00:00"}}},
            {"SSE", {{"9:30:00", "13:00:00"}, {"11:30:00", "15:00:00"}}},
            {"SZE", {{"9:30:00", "13:00:00"}, {"11:30:00", "15:00:00"}}}
        };

        std::map<std::string, bool> trading_evening {
            {"CZCE", true},
            {"SHFE", true},
            {"DCE", true},
            {"CFFEX", false},
            {"SSE", false},
            {"SZE", false}
        };
        int                             req_socket_;
        int                             sub_socket_;
        std::shared_ptr<std::thread>    thread_;
        std::atomic<bool>               started_;
        std::atomic<int>                current_;
        std::vector<SwitchDayCallback>  cbs_;
    };
    typedef std::shared_ptr<Calendar> CalendarPtr;
}

#endif //KUNGFU_CALENDAR_H
