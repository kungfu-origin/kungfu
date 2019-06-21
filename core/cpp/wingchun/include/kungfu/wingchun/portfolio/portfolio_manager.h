//
// Created by PolarAir on 2019-04-18.
//

#ifndef WINGCHUN_PORTFOLIO_MANAGER_H
#define WINGCHUN_PORTFOLIO_MANAGER_H

#include <kungfu/wingchun/msg.h>

#include "pnl_def.h"
#include "account_manager.h"

namespace kungfu
{
    namespace wingchun
    {
        class PortfolioManager : public IPnLDataHandler
        {
        public:
            PortfolioManager(const std::string& name);

            virtual msg::data::Position get_long_pos(const std::string& account_id, const std::string& instrument_id, const std::string& exchange_id);

            virtual msg::data::Position get_short_pos(const std::string& account_id, const std::string& instrument_id, const std::string& exchange_id);

            virtual double get_last_price(const std::string& instrument_id, const std::string& exchange_id);

            virtual std::vector<msg::data::Instrument> get_all_pos_instruments(const std::string& account_id);

            virtual msg::data::SubPortfolioInfo get_sub_portfolio(const std::string& account_id);

            virtual msg::data::PortfolioInfo get_portfolio();

            virtual const AccountManager& get_account(const std::string& account_id);

            virtual void dump_to_db(SQLite::Database &db) = 0;

            virtual void dump_to_db(const std::string& db_file) = 0;

            virtual void load_from_db(const std::string& db_file) = 0;

            virtual void load_from_db(SQLite::Database &db) = 0;

        private:
            bool recalc_pnl();

            void callback() const;

            void on_acc_callback(const msg::data::AccountInfo &acc) const;

            void on_pos_callback(const msg::data::Position &pos) const;

        private:
            std::string name_;
            int64_t last_update_;
            std::string trading_day_;
            std::map<std::string, AccountManager&> accounts_;
            msg::data::PortfolioInfo pnl_;
            std::vector<PnLCallback> cbs_;
            std::vector<AccountCallback> acc_cbs_;
            std::vector<PositionCallback> pos_cbs_;
        };
    }
}

#endif //WINGCHUN_PORTFOLIO_MANAGER_H
