//
// Created by qlu on 2019/5/21.
//

#ifndef PROJECT_PORTFOLIO_MANAGER_IMPL_H
#define PROJECT_PORTFOLIO_MANAGER_IMPL_H

#include <kungfu/wingchun/portfolio/portfolio_manager.h>

namespace kungfu
{
    class PortfolioManagerImpl: public PortfolioManager
    {
    public:
        PortfolioManagerImpl(const char* name);

        Position get_long_pos(const char* account_id, const char* instrument_id, const char* exchange_id) const override ;
        Position get_short_pos(const char* account_id, const char* instrument_id, const char* exchange_id) const override;
        double get_last_price(const char* instrument_id, const char* exchange_id) const override;
        std::vector<Instrument> get_all_pos_instruments(const char* account_id) const override;

        SubPortfolioInfo get_sub_portfolio(const char* account_id) const override;
        PortfolioInfo get_portfolio() const override;

        const AccountManagerPtr get_account(const char* account_id) const override;

        void on_quote(const Quote* quote) override;
        void on_order(const Order* order) override;
        void on_trade(const Trade* trade) override;
        void on_positions(const std::vector<Position>& positions) override;
        void on_position_details(const std::vector<Position>& details) override;
        void on_account(const AccountInfo& account) override;
        void on_order_input(const OrderInput* input) override;
        void on_switch_day(const std::string& trading_day) override;

        int64_t get_last_update() const override;
        std::string get_current_trading_day() const override;
        void set_current_trading_day(const std::string& trading_day) override;

        void register_pos_callback(PositionCallback cb) override;
        void register_acc_callback(AccountCallback cb) override;
        void register_pnl_callback(PnLCallback cb) override;

        virtual void dump_to_db(SQLite::Database& db) override;
        virtual void dump_to_db(const char* db_file) override;

        virtual void load_from_db(const char* db_file) override;
        virtual void load_from_db(SQLite::Database& db) override;

    private:
        bool recalc_pnl();
        void callback() const;
        void on_acc_callback(const AccountInfo& acc) const;
        void on_pos_callback(const Position& pos) const;

    private:
        std::string                                     name_;
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        std::map<std::string, AccountManagerPtr>        accounts_;
        PortfolioInfo                                   pnl_;
        std::vector<PnLCallback>                        cbs_;
        std::vector<AccountCallback>                    acc_cbs_;
        std::vector<PositionCallback>                   pos_cbs_;
    };
}
#endif //PROJECT_PORTFOLIO_MANAGER_IMPL_H
