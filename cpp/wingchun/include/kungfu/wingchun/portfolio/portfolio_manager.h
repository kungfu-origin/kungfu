//
// Created by PolarAir on 2019-04-18.
//

#ifndef KUNGFU_PORTFOLIO_MANAGER_H
#define KUNGFU_PORTFOLIO_MANAGER_H

#include "pnl_def.h"
#include "account_manager.h"

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    class PortfolioManager: public IPnLDataHandler
    {
    public:
        virtual void on_order_input(const OrderInput* input) = 0;
        virtual void on_quote(const Quote* quote) = 0;
        virtual void on_order(const Order* order) = 0;
        virtual void on_trade(const Trade* trade) = 0;

        virtual void on_positions(const std::vector<Position>& positions) = 0;
        virtual void on_position_details(const std::vector<Position>& details) = 0;
        virtual void on_account(const AccountInfo& account) = 0;

        virtual void on_switch_day(const std::string& trading_day) = 0;

        virtual int64_t get_last_update() const = 0;

        virtual std::string get_current_trading_day() const = 0;
        virtual void set_current_trading_day(const std::string& trading_day) = 0;

        virtual void register_pos_callback(PositionCallback cb) = 0;
        virtual void register_acc_callback(AccountCallback cb) = 0;
        virtual void register_pnl_callback(PnLCallback cb) = 0;

        virtual Position get_long_pos(const char* account_id, const char* instrument_id, const char* exchange_id) const = 0;
        virtual Position get_short_pos(const char* account_id, const char* instrument_id, const char* exchange_id) const = 0;
        virtual double get_last_price(const char* instrument_id, const char* exchange_id) const = 0;
        virtual std::vector<Instrument> get_all_pos_instruments(const char* account_id) const = 0;

        virtual SubPortfolioInfo get_sub_portfolio(const char* account_id) const = 0;
        virtual PortfolioInfo get_portfolio() const = 0;

        virtual const AccountManagerPtr get_account(const char* account_id) const = 0;

        virtual void dump_to_db(SQLite::Database& db) = 0;
        virtual void dump_to_db(const char* db_file) = 0;

        virtual void load_from_db(const char* db_file) = 0;
        virtual void load_from_db(SQLite::Database& db) = 0;
    };

    typedef std::shared_ptr<PortfolioManager> PortfolioManagerPtr;
    PortfolioManagerPtr create_portfolio_manager(const char* name, const char *db = nullptr);
}

#endif //KUNGFU_PORTFOLIO_MANAGER_H
