//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_ACCOUNT_MANAGER_H
#define KUNGFU_ACCOUNT_MANAGER_H

#include <kungfu/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/commission/commission_manager.h>

#include "pnl_def.h"
#include "position_manager.h"

namespace kungfu
{
    namespace wingchun
    {
        class AccountManager : public IPnLDataHandler
        {
        public:
            AccountManager(const std::string& account_id, Calendar& calendar, const std::string& commission_db_file);

            virtual void on_quote(const msg::data::Quote *quote);

            virtual void on_order_input(const msg::data::OrderInput *input);

            virtual void on_order(const msg::data::Order *order);

            virtual void on_trade(const msg::data::Trade *trade);

            virtual void on_positions(const std::vector<msg::data::Position> &positions);

            virtual void on_position_details(const std::vector<msg::data::Position> &details);

            virtual void on_account(const msg::data::AccountInfo &account);

            virtual void on_switch_day(const std::string &trading_day);

            virtual int64_t get_last_update() const
            { return last_update_; };

            virtual std::string get_current_trading_day() const
            { return trading_day_; };

            virtual void set_current_trading_day(const std::string &trading_day);

            virtual void register_pos_callback(PositionCallback cb);

            virtual void register_acc_callback(AccountCallback cb);

            virtual void register_pnl_callback(PnLCallback cb);

            virtual msg::data::AccountInfo get_account_info() const
            { return account_; };

            virtual msg::data::Position get_long_pos(const std::string& instrument_id, const std::string& exchange_id) const;

            virtual msg::data::Position get_short_pos(const std::string& instrument_id, const std::string& exchange_id) const;

            virtual std::vector<msg::data::Instrument> get_all_pos_instruments() const;

            virtual double get_last_price(const std::string& instrument_id, const std::string& exchange_id) const;

            virtual void set_initial_equity(double equity);

            virtual void set_static_equity(double equity);

            virtual double calc_commission(const msg::data::Trade *trade) const;

            virtual double calc_tax(const msg::data::Trade *trade) const;

            virtual void dump_to_db(SQLite::Database &db, bool save_meta = true);

            virtual void dump_to_db(const std::string& db_file, bool save_meta = true);

            virtual void load_from_db(SQLite::Database &db);

        private:
            bool recalc_acc();

            void callback() const;

            void on_pos_callback(const msg::data::Position &pos) const;

            Calendar& calendar_;
            int64_t last_update_;
            std::string trading_day_;

            CommissionManager commission_manager_;
            PositionManager position_manager_;

            msg::data::AccountInfo account_;

            std::map<std::string, double> bond_map_;
            std::map<int64_t, double> frozen_map_; // { order_id -> frozen_money }
            std::vector<AccountCallback> cbs_;
            std::vector<PositionCallback> pos_cbs_;
        };

        DECLARE_PTR(AccountManager)
    }
}

#endif //KUNGFU_ACCOUNT_MANAGER_H
