//
// Created by qlu on 2019/5/21.
//

#ifndef PROJECT_ACCOUNT_MANAGER_IMPL_H
#define PROJECT_ACCOUNT_MANAGER_IMPL_H

#include <kungfu/wingchun/portfolio/account_manager.h>
#include <kungfu/wingchun/portfolio/position_manager.h>
#include <kungfu/wingchun/commission/commission_manager.h>

namespace kungfu
{
    class AccountManagerImpl: public AccountManager
    {
    public:
        friend bool save(const std::string& db_file, const AccountManagerImpl* account_manager);
        friend bool load(const std::string& db_file, AccountManagerImpl* account_manager);
        friend bool save(SQLite::Database& db, const AccountManagerImpl* account_manager);
        friend bool load(SQLite::Database& db, AccountManagerImpl* account_manager);

        AccountManagerImpl(const char* account_id, AccountType type);

        AccountInfo get_account_info() const { return account_; };

        Position get_long_pos(const char* instrument_id, const char* exchange_id) const;
        Position get_short_pos(const char* instrument_id, const char* exchange_id) const;
        std::vector<Instrument> get_all_pos_instruments() const;

        double get_last_price(const char* instrument_id, const char* exchange_id) const;

        void on_quote(const Quote* quote);
        void on_order_input(const OrderInput* input);
        void on_order(const Order* order);
        void on_trade(const Trade* trade);

        void on_positions(const std::vector<Position>& positions);
        void on_position_details(const std::vector<Position>& details);
        void on_account(const AccountInfo& account);

        void on_switch_day(const std::string& trading_day);

        int64_t get_last_update() const { return last_update_; };

        std::string get_current_trading_day() const { return trading_day_; };
        void set_current_trading_day(const std::string& trading_day);

        void register_pos_callback(PositionCallback cb);
        void register_acc_callback(AccountCallback cb);
        void register_pnl_callback(PnLCallback cb);

        void set_initial_equity(double equity);
        void set_static_equity(double equity);

        double calc_commission(const Trade* trade) const;
        double calc_tax(const Trade* trade) const ;

        void dump_to_db(SQLite::Database& db, bool save_meta = true);
        void dump_to_db(const char* db_file, bool save_meta = true);

        void load_from_db(const char* db_file);
        void load_from_db(SQLite::Database& db);

    private:
        bool recalc_acc();
        void callback() const;
        void on_pos_callback(const Position& pos) const;

    private:
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        CommissionManager                               commission_;
        PositionManagerPtr                              pos_manager_;
        AccountInfo                                     account_;
        std::map<std::string, double>                   bond_map_;
        std::map<int64_t, double>                       frozen_map_; // { order_id -> frozen_money }
        std::vector<AccountCallback>                    cbs_;
        std::vector<PositionCallback>                   pos_cbs_;
    };
}

#endif //PROJECT_ACCOUNT_MANAGER_IMPL_H
