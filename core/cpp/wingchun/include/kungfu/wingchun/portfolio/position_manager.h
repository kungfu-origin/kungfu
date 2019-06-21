//
// Created by PolarAir on 2019-04-16.
//

#ifndef KUNGFU_POSITION_MANAGER_H
#define KUNGFU_POSITION_MANAGER_H

#include "pnl_def.h"

#include <kungfu/yijinjing/io.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        class PositionManager : public IPnLDataHandler
        {
        public:
            PositionManager(const std::string &account_id, Calendar &calendar);

            virtual msg::data::Position get_long_pos(const std::string &instrument_id, const std::string &exchange_id) const;

            virtual msg::data::Position get_short_pos(const std::string &instrument_id, const std::string &exchange_id) const;

            virtual std::vector<msg::data::Instrument> get_all_pos_instruments() const;

            virtual double get_last_price(const std::string &instrument_id, const std::string &exchange_id) const;

            virtual double get_market_value() const; // stock only

            void on_order_input(const msg::data::OrderInput *input);

            void on_quote(const msg::data::Quote *quote);

            void on_order(const msg::data::Order *order);

            void on_trade(const msg::data::Trade *trade);

            void on_positions(const std::vector<msg::data::Position> &positions);

            void on_position_details(const std::vector<msg::data::Position> &details);

            void on_account(const msg::data::AccountInfo &account);

            void on_switch_day(const std::string &trading_day);

            int64_t get_last_update() const;

            std::string get_current_trading_day() const;

            void set_current_trading_day(const std::string &trading_day);

            void register_pos_callback(PositionCallback cb);

            void register_acc_callback(AccountCallback cb);

            void register_pnl_callback(PnLCallback cb);

            virtual void dump_to_db(SQLite::Database &db);

            virtual void load_from_db(SQLite::Database &db);

        private:
            double choose_price(const msg::data::Position &pos) const;

            bool recalc_pos_by_price(msg::data::Position &pos);

            void callback(const msg::data::Position &pos) const;

            void init_pos_from_input(const msg::data::OrderInput *input);

            void init_pos_from_trade(const msg::data::Trade *trade);

            void deal_bond_from_buy(const msg::data::Trade *trade);

            void insert_detail_from_trade(const msg::data::Trade *trade);

            void remove_detail_from_trade(const msg::data::Trade *trade);

            void on_order_stock(const msg::data::Order *order);

            void on_order_future(const msg::data::Order *order);

            void on_trade_stock(const msg::data::Trade *trade);

            void on_trade_future(const msg::data::Trade *trade);

        private:
            std::string account_id_;
            Calendar &calendar_;
            int64_t last_update_;
            std::string trading_day_;
            std::map<std::string, msg::data::Position> long_pos_map_; // { symbol -> pos }
            std::map<std::string, msg::data::Position> short_pos_map_; // { symbol -> pos }
            std::map<std::string, std::vector<msg::data::Position>> bond_map_;  // { expire_date -> [ bond_pos ] }
            std::map<std::string, std::vector<msg::data::Position>> long_detail_map_; // { symbol -> [ detail ] }
            std::map<std::string, std::vector<msg::data::Position>> short_detail_map_; // { symbol -> [ detail ] }
            std::map<int64_t, int64_t> frozen_map_;// { order_id -> frozen_vol }
            std::vector<PositionCallback> cbs_;
        };

        DECLARE_PTR(PositionManager)
    }
}

#endif //KUNGFU_POSITION_MANAGER_H
