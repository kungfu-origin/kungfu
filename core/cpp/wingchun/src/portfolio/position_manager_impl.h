//
// Created by qlu on 2019/5/21.
//

#ifndef PROJECT_POSITION_MANAGER_IMPL_H
#define PROJECT_POSITION_MANAGER_IMPL_H

#include <kungfu/wingchun/portfolio/position_manager.h>

namespace kungfu
{
    class PositionManagerImpl: public PositionManager
    {
    public:
        PositionManagerImpl(const char* account_id);

        Position get_long_pos(const char* instrument_id, const char* exchange_id) const ;
        Position get_short_pos(const char* instrument_id, const char* exchange_id) const ;
        std::vector<Instrument> get_all_pos_instruments() const;
        double get_last_price(const char* instrument_id, const char* exchange_id) const;
        double get_market_value() const; // stock only

        void on_order_input(const OrderInput* input);
        void on_quote(const Quote* quote);
        void on_order(const Order* order);
        void on_trade(const Trade* trade);

        void on_positions(const std::vector<Position>& positions);
        void on_position_details(const std::vector<Position>& details);
        void on_account(const AccountInfo& account);

        void on_switch_day(const std::string& trading_day);

        int64_t get_last_update() const;
        std::string get_current_trading_day() const;
        void set_current_trading_day(const std::string& trading_day);

        void register_pos_callback(PositionCallback cb);
        void register_acc_callback(AccountCallback cb);
        void register_pnl_callback(PnLCallback cb);

        virtual void dump_to_db(SQLite::Database& db);
        virtual void dump_to_db(const char* db_file);

        virtual void load_from_db(const char* db_file);
        virtual void load_from_db(SQLite::Database& db);

    private:
        double choose_price(const Position& pos) const;
        bool recalc_pos_by_price(Position& pos);
        void callback(const Position& pos) const;
        void init_pos_from_input(const OrderInput* input);
        void init_pos_from_trade(const Trade* trade);
        void deal_bond_from_buy(const Trade* trade);
        void insert_detail_from_trade(const Trade* trade);
        void remove_detail_from_trade(const Trade* trade);
        void on_order_stock(const Order* order);
        void on_order_future(const Order* order);
        void on_trade_stock(const Trade* trade);
        void on_trade_future(const Trade* trade);

    private:
        std::string                                     account_id_;
        int64_t                                         last_update_;
        std::string                                     trading_day_;
        std::map<std::string, Position>                 long_pos_map_; // { symbol -> pos }
        std::map<std::string, Position>                 short_pos_map_; // { symbol -> pos }
        std::map<std::string, std::vector<Position>>    bond_map_;  // { expire_date -> [ bond_pos ] }
        std::map<std::string, std::vector<Position>>    long_detail_map_; // { symbol -> [ detail ] }
        std::map<std::string, std::vector<Position>>    short_detail_map_; // { symbol -> [ detail ] }
        std::map<int64_t, int64_t>                      frozen_map_;// { order_id -> frozen_vol }
        std::vector<PositionCallback>                   cbs_;
    };
}
#endif //PROJECT_POSITION_MANAGER_IMPL_H
