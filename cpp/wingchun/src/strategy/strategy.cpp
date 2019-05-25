//
// Created by qlu on 2019/1/29.
//

#include <functional>
#include <fmt/format.h>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>

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
        impl(Strategy* strategy, const std::string& name) : strategy_(strategy), name_(name), event_loop_(new EventLoop(name)), util_(new StrategyUtil(name))
        {
            std::string rep_url = STRATEGY_REP_URL(name_);
            rsp_socket_ = std::shared_ptr<yijinjing::nanomsg::socket>(new yijinjing::nanomsg::socket(AF_SP, NN_REP));
            try
            {
                rsp_socket_->bind(rep_url.c_str());
            }
            catch(std::exception &e)
            {
                SPDLOG_ERROR("failed to bind to rep_url {}, exception: {}", rep_url.c_str(), e.what());
                abort();
            }
        }

        StrategyUtilPtr get_util() const { return util_; }

        bool register_algo_service()
        {
           //TODO
            return true;
        }

        bool add_md(const std::string& source_id)
        {
            event_loop_->subscribe_yjj_journal(MD_JOURNAL_FOLDER(source_id), MD_JOURNAL_NAME(source_id), yijinjing::getNanoTime());
            return get_util()->add_md(source_id);
        }

        bool add_account(const std::string& source_id, const std::string& account_id, const double cash_limit)
        {
            event_loop_->subscribe_yjj_journal(TD_JOURNAL_FOLDER(source_id, account_id), TD_JOURNAL_NAME(source_id, account_id), yijinjing::getNanoTime());
            event_loop_->register_reload_instruments_callback(std::bind(&StrategyUtil::reload_instruments, util_));
            auto gateway_name = "td_" + source_id + "_" + account_id;
            event_loop_->subscribe_nanomsg(GATEWAY_PUB_URL(gateway_name));
            return get_util()->add_account(source_id, account_id, cash_limit);
        }

        void register_nanotime_callback(int64_t nano, std::function<void (int64_t)> callback)
        {
            event_loop_->register_nanotime_callback(nano, callback);
        }

        void run()
        {
            QuoteCallback quote_callback = std::bind(&Strategy::impl::process_quote, this, std::placeholders::_1);
            EntrustCallback entrust_callback = std::bind(&Strategy::on_entrust, strategy_, std::placeholders::_1);
            TransactionCallback transaction_callback = std::bind(&Strategy::on_transaction, strategy_, std::placeholders::_1);

            OrderCallback order_callback = std::bind(&Strategy::impl::process_order, this, std::placeholders::_1);
            TradeCallback trade_callback = std::bind(&Strategy::impl::process_trade, this, std::placeholders::_1);
            AlgoOrderStatusCallback algo_order_status_callback = std::bind(&Strategy::impl::process_algo_order_status, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

            event_loop_->register_nanotime_callback(nseconds_next_min(yijinjing::getNanoTime()), std::bind(&Strategy::impl::on_1min_timer, this, std::placeholders::_1));
            event_loop_->register_nanotime_callback(nseconds_next_day(yijinjing::getNanoTime()), std::bind(&Strategy::impl::on_daily_timer, this, std::placeholders::_1));

            event_loop_->register_quote_callback(quote_callback);
            event_loop_->register_entrust_callback(entrust_callback);
            event_loop_->register_transaction_callback(transaction_callback);

            event_loop_->register_order_callback(order_callback);
            event_loop_->register_trade_callback(trade_callback);

            event_loop_->register_algo_order_status_callback(algo_order_status_callback);

            event_loop_->register_manual_order_action_callback(std::bind(&Strategy::impl::on_manual_order_action, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            event_loop_->register_signal_callback(std::bind(&Strategy::pre_quit, strategy_));


            strategy_->pre_run();

            event_loop_->run();
        }

        void stop()
        {
            event_loop_->stop();
            strategy_->pre_quit();
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
            std::string js = to_string(msg);
            rsp_socket_->send(js, 0);
        }

        void on_1min_timer(int64_t nano)
        {
            util_->on_push_by_min();
            event_loop_->register_nanotime_callback(nano + yijinjing::NANOSECONDS_PER_MINUTE, std::bind(&Strategy::impl::on_1min_timer, this, std::placeholders::_1));
        }

        void on_daily_timer(int64_t nano)
        {
            util_->on_push_by_day();
            event_loop_->register_nanotime_callback(nano + yijinjing::NANOSECONDS_PER_DAY, std::bind(&Strategy::impl::on_daily_timer, this, std::placeholders::_1));
        }

        int64_t get_nano() const { return event_loop_->get_nano(); }

    private:
        Strategy* strategy_;
        std::string name_;
        EventLoopPtr event_loop_;
        StrategyUtilPtr util_;
        std::shared_ptr<yijinjing::nanomsg::socket> rsp_socket_;
    };

    Strategy::Strategy(const std::string& name) : name_(name), impl_(new impl(this, name)) {}

    Strategy::~Strategy() {}

    const std::string& Strategy::get_name()
    {
        return name_;
    }

    void Strategy::run()
    {
        impl_->run();
    }

    void Strategy::stop()
    {
        impl_->stop();
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

    void Strategy::register_nanotime_callback(int64_t nano, std::function<void (int64_t)> callback)
    {
        return impl_->register_nanotime_callback(nano, callback);
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
