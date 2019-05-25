//
// Created by qlu on 2019/1/18.
//

#ifndef PROJECT_STRATEGY_UTIL_H
#define PROJECT_STRATEGY_UTIL_H

#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <memory>
#include <set>

#include <kungfu/yijinjing/journal/journal.h>

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/util/uid_generator.h>
#include <kungfu/wingchun/storage/source_list_storage.h>
#include <kungfu/wingchun/storage/account_list_storage.h>
#include <kungfu/wingchun/gateway/util.hpp>
#include <kungfu/wingchun/calendar/calendar.h>

namespace kungfu
{
    namespace oms
    {
        class OrderManager;
    }
    class PortfolioManager;
    class NNPublisher;

    class StrategyUtil
    {
    public:
        explicit StrategyUtil(const std::string& name);
        ~StrategyUtil();

        bool add_md(const std::string& source_id);
        bool add_account(const std::string& source_id, const std::string& account_id, double cash_limit);
        bool register_algo_service();

        void on_quote(const journal::Quote& quote);
        void on_quote_py(uintptr_t quote);
        void on_order(const journal::Order& order);
        void on_order_py(uintptr_t order);
        void on_trade(const journal::Trade& trade);
        void on_trade_py(uintptr_t trade);
        void on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& status_msg);

        void register_switch_day_callback(std::function<void (const std::string&)> cb);

        void subscribe(const std::string& source, const std::vector<std::string>& instruments, const std::string& exchange_id, bool is_level2 = false);
        bool is_subscribed(const std::string& source, const std::string& instrument, const std::string& exchange_id) const;

        uint64_t insert_limit_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                              double limit_price, int64_t volume, Side side, Offset offset);
        uint64_t insert_fok_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                    double limit_price, int64_t volume, Side side, Offset offset);
        uint64_t insert_fak_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                    double limit_price, int64_t volume, Side side, Offset offset);
        uint64_t insert_market_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                     int64_t volume, Side side, Offset offset);
        uint64_t insert_order(journal::OrderInput& order_input);
        uint64_t cancel_order(uint64_t order_id);

        uint64_t insert_algo_order(const std::string& algo_type, const std::string& order_input_msg);
        uint64_t modify_algo_order(uint64_t order_id, const std::string& cmd);

        const journal::Quote* const get_last_md(const std::string& instrument_id, const std::string& exchange_id) const;
        flying::Position get_position(const std::string& instrument_id, const std::string& exchange_id, Direction direction = DirectionLong, const std::string& account_id = "") const;
        flying::PortfolioInfo get_portfolio_info() const;
        flying::SubPortfolioInfo get_sub_portfolio_info(const std::string& account_id) const;

        std::vector<uint64_t> get_pending_orders(const std::string& account_id = "") const;

        void on_push_by_min();
        void on_push_by_day();

        void reload_instruments();

    private:
        uint64_t next_id();
        std::vector<std::string> get_md_sources();
        std::vector<flying::SubPortfolioInfo> get_accounts();
        void on_switch_day(const std::string& trading_day);
        void init_portfolio_manager();

    private:
        std::string name_;
        CalendarPtr calendar_;
        std::shared_ptr<NNPublisher> publisher_;
        std::shared_ptr<PortfolioManager> portfolio_manager_;
        std::shared_ptr<oms::OrderManager> order_manager_;
        yijinjing::JournalWriterPtr writer_;
        std::unique_ptr<UidGenerator> uid_generator_;
        std::map<std::string, journal::Quote> quote_map_;
        bool has_stock_account_;
        bool has_future_account_;
        std::map<std::string, std::set<std::string>> subscribed_;
        std::vector<std::function<void (const std::string&)>> cbs_;
    };
    DECLARE_PTR(StrategyUtil)
}
#endif //PROJECT_STRATEGY_UTIL_H
