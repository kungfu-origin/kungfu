//
// Created by qlu on 2019/5/21.
//

#include "portfolio_manager_impl.h"
#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/serialize.h>
#include <spdlog/spdlog.h>
#include <map>
#include <boost/core/ignore_unused.hpp>
#include "storage.h"

namespace kungfu
{
    PortfolioManagerImpl::PortfolioManagerImpl(const char* name): name_(name), last_update_(0), trading_day_(""), pnl_({}) {}

    Position PortfolioManagerImpl::get_long_pos(const char *account_id, const char *instrument_id, const char *exchange_id) const
    {
        Position pos = {};
        strcpy(pos.instrument_id, instrument_id);
        strcpy(pos.exchange_id, exchange_id);
        strcpy(pos.client_id, name_.c_str());
        pos.instrument_type = get_instrument_type(instrument_id, exchange_id);
        pos.direction = DirectionLong;
        if (nullptr != account_id)
        {
            strcpy(pos.account_id, account_id);
        }
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                if (nullptr == account_id || strlen(account_id) == 0 || strcmp(account_id, iter.first.c_str()) == 0)
                {
                    auto cur_pos = iter.second->get_long_pos(instrument_id, exchange_id);
                    pos.update_time = std::max<int64_t>(pos.update_time, cur_pos.update_time);
                    pos.volume += cur_pos.volume;
                    pos.yesterday_volume += cur_pos.yesterday_volume;
                    pos.frozen_total += cur_pos.frozen_total;
                    pos.frozen_yesterday += cur_pos.frozen_yesterday;
                    pos.last_price = cur_pos.last_price;
                    pos.open_price = (pos.open_price * pos.volume + cur_pos.open_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                    pos.cost_price = (pos.cost_price * pos.volume + cur_pos.cost_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                    pos.close_price = cur_pos.close_price;
                    pos.pre_close_price = cur_pos.pre_close_price;
                    pos.settlement_price = cur_pos.settlement_price;
                    pos.pre_settlement_price = cur_pos.pre_settlement_price;
                    pos.margin += cur_pos.margin;
                    pos.position_pnl += cur_pos.position_pnl;
                    pos.close_pnl += cur_pos.close_pnl;
                    pos.realized_pnl += cur_pos.realized_pnl;
                    pos.unrealized_pnl += cur_pos.unrealized_pnl;
                }
            }
        }
        return pos;
    }

    Position PortfolioManagerImpl::get_short_pos(const char *account_id, const char *instrument_id, const char *exchange_id) const
    {
        Position pos = {};
        strcpy(pos.instrument_id, instrument_id);
        strcpy(pos.exchange_id, exchange_id);
        strcpy(pos.client_id, name_.c_str());
        pos.direction = DirectionShort;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                if (nullptr == account_id || strlen(account_id) == 0 || strcmp(account_id, iter.first.c_str()) == 0)
                {
                    auto cur_pos = iter.second->get_short_pos(instrument_id, exchange_id);
                    pos.volume += cur_pos.volume;
                    pos.yesterday_volume += cur_pos.yesterday_volume;
                    pos.frozen_total += cur_pos.frozen_total;
                    pos.frozen_yesterday += cur_pos.frozen_yesterday;
                    pos.last_price = cur_pos.last_price;
                    pos.open_price = (pos.open_price * pos.volume + cur_pos.open_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                    pos.cost_price = (pos.cost_price * pos.volume + cur_pos.cost_price * cur_pos.volume) / (pos.volume + cur_pos.volume);
                    pos.close_price = cur_pos.close_price;
                    pos.pre_close_price = cur_pos.pre_close_price;
                    pos.settlement_price = cur_pos.settlement_price;
                    pos.pre_settlement_price = cur_pos.pre_settlement_price;
                    pos.margin += cur_pos.margin;
                    pos.position_pnl += cur_pos.position_pnl;
                    pos.close_pnl += cur_pos.close_pnl;
                    pos.realized_pnl += cur_pos.realized_pnl;
                    pos.unrealized_pnl += cur_pos.unrealized_pnl;
                }
            }
        }
        return pos;
    }

    double PortfolioManagerImpl::get_last_price(const char *instrument_id, const char *exchange_id) const
    {
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                double price = iter.second->get_last_price(instrument_id, exchange_id);
                if (!is_zero(price))
                {
                    return price;
                }
            }
        }
        return 0;
    }

    std::vector<Instrument> PortfolioManagerImpl::get_all_pos_instruments(const char *account_id) const
    {
        std::map<std::string, Instrument> m;
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                if (nullptr == account_id || strlen(account_id) == 0 || strcmp(account_id, iter.first.c_str()) == 0)
                {
                    auto ins_vec = iter.second->get_all_pos_instruments();
                    for (const auto& pos : ins_vec)
                    {
                        auto key = get_symbol(pos.instrument_id, pos.exchange_id);
                        if (m.find(key) == m.end())
                        {
                            m[key] = pos;
                        }
                    }
                }
            }
        }
        std::vector<Instrument> ret;
        for (const auto& iter : m)
        {
            ret.emplace_back(iter.second);
        }
        return ret;
    }

    SubPortfolioInfo PortfolioManagerImpl::get_sub_portfolio(const char *account_id) const
    {
        if (accounts_.find(account_id) != accounts_.end())
        {
            return accounts_.at(account_id)->get_account_info();
        }
        else
        {
            SubPortfolioInfo subPortfolioInfo = {};
            strcpy(subPortfolioInfo.account_id, account_id);
            return subPortfolioInfo;
        }
    }

    PortfolioInfo PortfolioManagerImpl::get_portfolio() const
    {
        return pnl_;
    }

    const AccountManagerPtr PortfolioManagerImpl::get_account(const char *account_id) const
    {
        if (accounts_.find(account_id) != accounts_.end())
        {
            return accounts_.at(account_id);
        }
        return nullptr;
    }

#define IMPLEMENT_DATA_BODY(func_name, ...) \
    for (const auto& iter : accounts_) \
    { \
        if (nullptr != iter.second) \
        { \
            iter.second->func_name(__VA_ARGS__); \
        } \
    } \
    if (recalc_pnl()) \
    { \
        callback(); \
    }

    void PortfolioManagerImpl::on_quote(const kungfu::journal::Quote *quote)
    {
        last_update_ = quote->rcv_time;
        IMPLEMENT_DATA_BODY(on_quote, quote)
    }

    void PortfolioManagerImpl::on_order(const kungfu::journal::Order *order)
    {
        last_update_ = order->rcv_time;
        IMPLEMENT_DATA_BODY(on_order, order)
    }

    void PortfolioManagerImpl::on_trade(const kungfu::journal::Trade *trade)
    {
        SPDLOG_TRACE("trade: {}", journal::to_string(*trade));
        SPDLOG_TRACE("pnl before: {}", flying::to_string(pnl_));
        last_update_ = trade->rcv_time;
        IMPLEMENT_DATA_BODY(on_trade, trade)
        SPDLOG_TRACE("pnl after: {}", flying::to_string(pnl_));
    }

    void PortfolioManagerImpl::on_positions(const std::vector<kungfu::flying::Position> &positions)
    {
        if (positions.size() > 0)
        {
            last_update_ = std::max<int64_t>(last_update_, positions[0].rcv_time);
        }
        IMPLEMENT_DATA_BODY(on_positions, positions)
    }

    void PortfolioManagerImpl::on_position_details(const std::vector<kungfu::flying::Position> &details)
    {
        if (details.size() > 0)
        {
            last_update_ = std::max<int64_t>(last_update_, details[0].rcv_time);
        }
        IMPLEMENT_DATA_BODY(on_position_details, details)
    }

    void PortfolioManagerImpl::on_account(const kungfu::flying::AccountInfo &account)
    {
        last_update_ = std::max<int64_t>(last_update_, account.rcv_time);
        if (accounts_.find(account.account_id) == accounts_.end())
        {
            SPDLOG_INFO("portfolio {} add account{}", name_, account.account_id);
            auto account_manager = create_account_manager(account.account_id, account.account_type);
            account_manager->set_current_trading_day(trading_day_);
            accounts_[account.account_id] = account_manager;
            account_manager->register_pos_callback(std::bind(&PortfolioManagerImpl::on_pos_callback, this, std::placeholders::_1));
            account_manager->register_acc_callback(std::bind(&PortfolioManagerImpl::on_acc_callback, this, std::placeholders::_1));
        }
        accounts_[account.account_id]->on_account(account);
        SPDLOG_TRACE("pnl after on_account: {}", flying::to_string(pnl_));
    }

    void PortfolioManagerImpl::on_order_input(const kungfu::journal::OrderInput *input)
    {
        IMPLEMENT_DATA_BODY(on_order_input, input);
    }

    void PortfolioManagerImpl::on_switch_day(const std::string & trading_day)
    {
        if (trading_day <= trading_day_)
        {
            return;
        }
        trading_day_ = trading_day;
        strcpy(pnl_.trading_day, trading_day.c_str());

        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                iter.second->on_switch_day(trading_day);
            }
        }

        recalc_pnl();
        callback();
    }

    int64_t PortfolioManagerImpl::get_last_update() const
    {
        return last_update_;
    }

    std::string PortfolioManagerImpl::get_current_trading_day() const
    {
        return trading_day_;
    }

    void PortfolioManagerImpl::set_current_trading_day(const std::string &trading_day)
    {
        for (const auto& iter : accounts_)
        {
            if (nullptr != iter.second)
            {
                iter.second->set_current_trading_day(trading_day);
            }
        }

        if (trading_day >= trading_day_)
        {
            if (!trading_day_.empty())
            {
                on_switch_day(trading_day);
            }
            else
            {
                trading_day_ = trading_day;
                strcpy(pnl_.trading_day, trading_day.c_str());
            }
        }
    }

    void PortfolioManagerImpl::register_pos_callback(kungfu::PositionCallback cb)
    {
        pos_cbs_.emplace_back(std::move(cb));
    }

    void PortfolioManagerImpl::register_acc_callback(kungfu::AccountCallback cb)
    {
        acc_cbs_.emplace_back(std::move(cb));
    }

    void PortfolioManagerImpl::register_pnl_callback(kungfu::PnLCallback cb)
    {
        cbs_.emplace_back(std::move(cb));
    }

    bool PortfolioManagerImpl::recalc_pnl()
    {
        double old_dynamic = pnl_.dynamic_equity;

        pnl_.update_time = yijinjing::getNanoTime();
        strcpy(pnl_.trading_day, trading_day_.c_str());
        pnl_.initial_equity = 0;
        pnl_.static_equity = 0;
        pnl_.dynamic_equity = 0;
        for (const auto& iter : accounts_)
        {
            auto account_info = iter.second->get_account_info();
            pnl_.initial_equity += account_info.initial_equity;
            pnl_.static_equity += account_info.static_equity;
            pnl_.dynamic_equity += account_info.dynamic_equity;
        }
        pnl_.accumulated_pnl = pnl_.dynamic_equity - pnl_.initial_equity;
        pnl_.accumulated_pnl_ratio = is_zero(pnl_.initial_equity) ? 0 : pnl_.accumulated_pnl / pnl_.initial_equity;
        pnl_.intraday_pnl = pnl_.dynamic_equity - pnl_.static_equity;
        pnl_.intraday_pnl_ratio = is_zero(pnl_.static_equity) ? 0 : pnl_.intraday_pnl / pnl_.static_equity;
        return !is_equal(old_dynamic, pnl_.dynamic_equity);
    }

    void PortfolioManagerImpl::callback() const
    {
        for (const auto& cb : cbs_)
        {
            cb(pnl_);
        }
    }

    void PortfolioManagerImpl::on_pos_callback(const kungfu::flying::Position &pos) const
    {
        auto total_pos = pos.direction == DirectionLong ?
                         get_long_pos(nullptr, pos.instrument_id, pos.exchange_id) :
                         get_short_pos(nullptr, pos.instrument_id, pos.exchange_id);

        for (auto& cb : pos_cbs_)
        {
            cb(total_pos);
        }
    }

    void PortfolioManagerImpl::on_acc_callback(const kungfu::flying::AccountInfo &acc) const
    {
        for (auto& cb : acc_cbs_)
        {
            cb(acc);
        }
    }

    void PortfolioManagerImpl::load_from_db(SQLite::Database& db)
    {
        if (!db.tableExists("meta"))
        {
            return;
        }
        portfolio_util::load_meta_inner(db, last_update_, trading_day_);
        pnl_ = {};
        SQLite::Statement query_pnl(db, "SELECT * FROM portfolio");
        if (query_pnl.executeStep())
        {
            strcpy(pnl_.trading_day, query_pnl.getColumn(0));
            pnl_.update_time = query_pnl.getColumn(1);
            pnl_.initial_equity = query_pnl.getColumn(2);
            pnl_.static_equity = query_pnl.getColumn(3);
            pnl_.dynamic_equity = query_pnl.getColumn(4);
            pnl_.accumulated_pnl = query_pnl.getColumn(5);
            pnl_.accumulated_pnl_ratio = query_pnl.getColumn(6);
            pnl_.intraday_pnl = query_pnl.getColumn(7);
            pnl_.intraday_pnl_ratio = query_pnl.getColumn(8);
        }

        accounts_.clear();
        SQLite::Statement query_account(db, "SELECT account_id, type FROM account");
        while (query_account.executeStep())
        {
            std::string account_id = query_account.getColumn(0).getString();
            AccountType type = query_account.getColumn(1).getString()[0];
            auto acc_manager = create_account_manager(account_id.c_str(), type, db.getFilename().c_str());
            accounts_[account_id] = acc_manager;
            acc_manager->register_pos_callback(std::bind(&PortfolioManagerImpl::on_pos_callback, this, std::placeholders::_1));
            acc_manager->register_acc_callback(std::bind(&PortfolioManagerImpl::on_acc_callback, this, std::placeholders::_1));
        }
    }

    void PortfolioManagerImpl::load_from_db(const char* db_file)
    {
        portfolio_util::init_db(db_file);

        SQLite::Database db(db_file, SQLite::OPEN_READONLY);
        load_from_db(db);
    }

    void PortfolioManagerImpl::dump_to_db(SQLite::Database& db)
    {
        SQLite::Transaction transaction(db);
        db.exec("DELETE FROM portfolio");
        SQLite::Statement insert_portfolio(db, "INSERT INTO portfolio(trading_day, update_time, initial_equity, static_equity, dynamic_equity,"
                "accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio"
                ") VALUES(?,?,?,?,?,?,?,?,?)");
        insert_portfolio.bind(1, pnl_.trading_day);
        insert_portfolio.bind(2, pnl_.update_time);
        insert_portfolio.bind(3, pnl_.initial_equity);
        insert_portfolio.bind(4, pnl_.static_equity);
        insert_portfolio.bind(5, pnl_.dynamic_equity);
        insert_portfolio.bind(6, pnl_.accumulated_pnl);
        insert_portfolio.bind(7, pnl_.accumulated_pnl_ratio);
        insert_portfolio.bind(8, pnl_.intraday_pnl);
        insert_portfolio.bind(9, pnl_.intraday_pnl_ratio);
        insert_portfolio.exec();

        for (const auto& acc_iter : accounts_)
        {
            if (nullptr != acc_iter.second)
            {
                acc_iter.second->dump_to_db(db, false);
            }
        }
        portfolio_util::save_meta_inner(db, last_update_, trading_day_);
        transaction.commit();
    }

    void PortfolioManagerImpl::dump_to_db(const char* db_file)
    {
        SQLite::Database db(db_file, SQLite::OPEN_READWRITE);
        dump_to_db(db);
    }

    PortfolioManagerPtr create_portfolio_manager(const char* name, const char *db)
    {
        auto portfolio_manager = PortfolioManagerPtr(new PortfolioManagerImpl(name));
        if (db != nullptr)
        {
            portfolio_manager->load_from_db(db);
        }
        return portfolio_manager;
    }
}
