//
// Created by qlu on 2019/1/29.
//

#include <functional>
#include <fmt/format.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/strategy/strategy.h>
#include <kungfu/wingchun/strategy/strategy_util.h>
#include <kungfu/wingchun/event_loop/event_loop.h>
#include <kungfu/wingchun/msg.h>

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/util/timer_util.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    class Strategy::impl
    {
    public:
        impl(Strategy* strategy, const std::string& name) : strategy_(strategy), name_(name), util_(new StrategyUtil(strategy, name)) {}

        StrategyUtil_ptr get_util() const { return util_; }

        bool register_algo_service()
        {
           //TODO
            return true;
        }

        bool add_md(const std::string& source_id)
        {
            SPDLOG_INFO("add md {}", source_id);
            strategy_->event_source_->subscribe(std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::MD, source_id, source_id));
            return get_util()->add_md(source_id);
        }

        bool add_account(const std::string& source_id, const std::string& account_id, const double cash_limit)
        {
            SPDLOG_INFO("add account {}/{}", source_id, account_id);
            strategy_->register_reload_instruments_callback(std::bind(&StrategyUtil::reload_instruments, util_));
            strategy_->event_source_->subscribe(std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::TD, source_id, account_id));
            return get_util()->add_account(source_id, account_id, cash_limit);
        }

        void init()
        {
            QuoteCallback quote_callback = std::bind(&Strategy::impl::process_quote, this, std::placeholders::_1);
            EntrustCallback entrust_callback = std::bind(&Strategy::on_entrust, strategy_, std::placeholders::_1);
            TransactionCallback transaction_callback = std::bind(&Strategy::on_transaction, strategy_, std::placeholders::_1);

            OrderCallback order_callback = std::bind(&Strategy::impl::process_order, this, std::placeholders::_1);
            TradeCallback trade_callback = std::bind(&Strategy::impl::process_trade, this, std::placeholders::_1);
            AlgoOrderStatusCallback algo_order_status_callback = std::bind(&Strategy::impl::process_algo_order_status, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            int64_t now = yijinjing::time::now_in_nano();
            strategy_->register_nanotime_callback(yijinjing::time::next_minute_nano(now), std::bind(&Strategy::impl::on_1min_timer, this, std::placeholders::_1));
            strategy_->register_nanotime_callback(yijinjing::time::next_day_nano(now), std::bind(&Strategy::impl::on_daily_timer, this, std::placeholders::_1));

            strategy_->register_quote_callback(quote_callback);
            strategy_->register_entrust_callback(entrust_callback);
            strategy_->register_transaction_callback(transaction_callback);

            strategy_->register_order_callback(order_callback);
            strategy_->register_trade_callback(trade_callback);

            strategy_->register_algo_order_status_callback(algo_order_status_callback);

            strategy_->register_manual_order_action_callback(std::bind(&Strategy::impl::on_manual_order_action, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        }

        void process_quote(const Quote& quote)
        {
            if (util_->is_subscribed(quote.source_id, quote.instrument_id, quote.exchange_id))
            {
                util_->on_quote(quote);
                strategy_->on_quote(quote);
            }            
        }
        void process_order(const Order& order)
        {
            if (name_ == order.client_id)
            {
                util_->on_order(order);
                strategy_->on_order(order);
            }
        }
        void process_trade(const Trade& trade)
        {
            if (name_ == trade.client_id)
            {
                util_->on_trade(trade);
                strategy_->on_trade(trade);
            }
        }
        void process_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& status_msg)
        {
            util_->on_algo_order_status(order_id, algo_type, status_msg);
            strategy_->on_algo_order_status(order_id, algo_type, status_msg);
        }

        void on_manual_order_action(const std::string& account_id, const std::string& client_id, const std::vector<uint64_t>& order_ids)
        {
            int error_id = 0;
            std::string error_text = "";
            int cancel_count = 0;
            if (!order_ids.empty())
            {
                for (const auto& order_id : order_ids)
                {
                    util_->cancel_order(order_id);
                }
                cancel_count = order_ids.size();
            }
            else
            {
                auto pending_orders = util_->get_pending_orders(account_id);
                for (const auto& order_id : pending_orders)
                {
                    util_->cancel_order(order_id);
                }
                cancel_count = pending_orders.size();
            }

            NNMsg msg = {MsgType::RspOrderAction, {}};
            msg.data["error_id"] = error_id;
            msg.data["error_text"] = error_text;
            msg.data["cancel_count"] = cancel_count;

            nlohmann::json j = msg;
            j["gen_time"] = yijinjing::time::now_in_nano();
            j["trigger_time"] = yijinjing::time::now_in_nano();
            j["source"] = 0;
            strategy_->event_source_->get_socket_reply()->send_json(j);
        }

        void on_1min_timer(int64_t nano)
        {
            util_->on_push_by_min();
            strategy_->register_nanotime_callback(nano + yijinjing::time_unit::NANOSECONDS_PER_MINUTE, std::bind(&Strategy::impl::on_1min_timer, this, std::placeholders::_1));
        }

        void on_daily_timer(int64_t nano)
        {
            util_->on_push_by_day();
            strategy_->register_nanotime_callback(nano + yijinjing::time_unit::NANOSECONDS_PER_DAY, std::bind(&Strategy::impl::on_daily_timer, this, std::placeholders::_1));
        }

        int64_t get_nano() const { return strategy_->get_nano(); }

    private:
        Strategy* strategy_;
        std::string name_;
        StrategyUtil_ptr util_;
    };

    Strategy::Strategy(const std::string& name) : name_(name) {
        kungfu::yijinjing::log::copy_log_settings(name);
        impl_ = std::make_unique<impl>(this, name);
    }

    Strategy::~Strategy() {}

    const std::string& Strategy::get_name() const
    {
        return name_;
    }

    void Strategy::configure_event_source(kungfu::yijinjing::event_source_ptr event_source)
    {
        event_source_ = event_source;
        impl_->get_util()->init(event_source);
        impl_->init();
        SPDLOG_INFO("Strategy pre run");
        pre_run();
    }

    void Strategy::finish()
    {
        SPDLOG_INFO("Strategy pre quit");
        pre_quit();
    }

    bool Strategy::add_md(const std::string& source_id)
    {
        return impl_->add_md(source_id);
    }

    bool Strategy::add_account(const std::string &source_id, const std::string &account_id, double cash_limit)
    {
        return impl_->add_account(source_id, account_id, cash_limit);
    }

    bool Strategy::register_algo_service()
    {
        return impl_->register_algo_service();
    }

    int64_t Strategy::get_nano() const { return impl_->get_nano(); }

    const Quote* const Strategy::get_last_md(const std::string& instrument_id, const std::string& exchange_id) const
    {
        return impl_->get_util()->get_last_md(instrument_id, exchange_id);
    }

    Position Strategy::get_position(const std::string& instrument_id, const std::string& exchange_id, Direction direction, const std::string& account_id) const
    {
        return impl_->get_util()->get_position(instrument_id, exchange_id, direction, account_id);
    }

    PortfolioInfo Strategy::get_portfolio_info() const
    {
        return impl_->get_util()->get_portfolio_info();
    }

    SubPortfolioInfo Strategy::get_sub_portfolio_info(const std::string& account_id) const
    {
        return impl_->get_util()->get_sub_portfolio_info(account_id);
    }

    void Strategy::subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string& exchange_id, bool is_level2)
    {
        impl_->get_util()->subscribe(source, instruments, exchange_id, is_level2);
    }

    uint64_t Strategy::insert_limit_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                double limit_price, int64_t volume, Side side, Offset offset)
    {
        return impl_->get_util()->insert_limit_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset);
    }

    uint64_t Strategy::insert_fok_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                              double limit_price, int64_t volume, Side side, Offset offset)
    {
        return impl_->get_util()->insert_fok_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset);
    }

    uint64_t Strategy::insert_fak_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                              double limit_price, int64_t volume, Side side, Offset offset)
    {
        return impl_->get_util()->insert_fak_order(instrument_id, exchange_id, account_id, limit_price, volume, side, offset);
    }

    uint64_t Strategy::insert_market_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id, int64_t volume, Side side, Offset offset)
    {
        return impl_->get_util()->insert_market_order(instrument_id, exchange_id, account_id, volume, side, offset);
    }

    uint64_t Strategy::cancel_order(uint64_t order_id)
    {
        return impl_->get_util()->cancel_order(order_id);
    }

    uint64_t Strategy::insert_algo_order(const std::string &algo_type, const std::string &algo_order_input)
    {
        return impl_->get_util()->insert_algo_order(algo_type, algo_order_input);
    }

    uint64_t Strategy::modify_algo_order(uint64_t order_id, const std::string &cmd)
    {
        return impl_->get_util()->modify_algo_order(order_id, cmd);
    }

    uint64_t Strategy::try_frozen(const std::string &account_id, const AssetsFrozen &frozen)
    {
        //TODO
        return 0;
    }

    bool Strategy::try_frozen(uint64_t op_id, const std::string &account_id, const AssetsFrozen &frozen)
    {
        //TODO
        return false;
    }

    void Strategy::cancel_frozen(uint64_t op_id, const std::string& account_id)
    {
        //TODO
    }

    bool Strategy::commit_frozen(uint64_t op_id, const std::string& account_id)
    {
        //TODO
        return false;
    }

    uint64_t Strategy::try_modify_position(const std::string& account_id, const PositionModify& pos_modify)
    {
        //TODO
        return 0;
    }

    void Strategy::cancel_modify_position(uint64_t op_id, const std::string& account_id)
    {
        //TODO
    }

    bool Strategy::commit_modify_position(uint64_t op_id, const std::string& account_id)
    {
        //TODO
        return false;
    }
}
