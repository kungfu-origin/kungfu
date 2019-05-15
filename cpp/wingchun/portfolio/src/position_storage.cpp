//
// Created by PolarAir on 2019-04-16.
//

#include "position_storage.h"
#include "storage_common.h"
#include "position_manager.hpp"
#include "util/include/business_helper.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    PositionStorage::PositionStorage(const char *account_id) : account_id_(account_id)
    {

    }

    bool PositionStorage::save(SQLite::Database &db, const kungfu::PositionManager *pos_manager, bool save_meta)
    {
        if (nullptr == pos_manager)
            return false;

        try
        {
            db.exec(fmt::format("DELETE FROM position WHERE account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM bond_position WHERE account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM pos_detail WHERE account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM pos_frozen WHERE account_id = '{}'", account_id_));

            for (const auto &iter : pos_manager->impl_->long_pos_map_) {
                save_single_pos(db, "position", iter.second);
            }
            for (const auto &iter : pos_manager->impl_->short_pos_map_) {
                save_single_pos(db, "position", iter.second);
            }

            for (const auto &iter : pos_manager->impl_->bond_map_) {
                for (const auto &bond : iter.second) {
                    save_single_pos(db, "bond_position", bond);
                }
            }

            for (const auto &iter : pos_manager->impl_->long_detail_map_) {
                for (const auto &detail : iter.second) {
                    save_single_pos(db, "pos_detail", detail);
                }
            }
            for (const auto &iter : pos_manager->impl_->short_detail_map_) {
                for (const auto &detail : iter.second) {
                    save_single_pos(db, "pos_detail", detail);
                }
            }

            for (const auto &iter : pos_manager->impl_->frozen_map_) {
                db.exec(fmt::format("INSERT INTO pos_frozen(order_id, account_id, frozen_vol) VALUES({}, '{}', {})",
                                    iter.first, account_id_, iter.second));
            }

            if (save_meta)
                save_meta_inner(db, pos_manager->impl_->last_update_, pos_manager->impl_->trading_day_);

            return true;
        } catch (std::exception &e)
        {
            SPDLOG_ERROR("failed to save position to db, {}", e.what());
            return false;
        }
    }

    void PositionStorage::load(SQLite::Database &db, kungfu::PositionManager *pos_manager)
    {
        db.exec("BEGIN");
        try {
            load_meta_inner(db, pos_manager->impl_->last_update_, pos_manager->impl_->trading_day_);

            auto pos_func = [&](std::map<std::string, Position> &pos_map, Direction direction) {
                pos_map.clear();
                SQLite::Statement query_pos(db, "SELECT * FROM position WHERE account_id = ? and direction = ?");
                query_pos.bind(1, account_id_);
                query_pos.bind(2, std::string(1, direction));
                Position pos = {};
                while (query_pos.executeStep()) {
                    load_single_pos(query_pos, pos);
                    pos_map[get_symbol(pos.instrument_id, pos.exchange_id)] = pos;
                }
            };
            pos_func(pos_manager->impl_->long_pos_map_, DirectionLong);
            pos_func(pos_manager->impl_->short_pos_map_, DirectionShort);

            pos_manager->impl_->bond_map_.clear();
            SQLite::Statement query_bond(db, "SELECT * FROM bond_position WHERE account_id = ?");
            query_bond.bind(1, account_id_);
            Position bond = {};
            while (query_bond.executeStep()) {
                load_single_pos(query_bond, bond);
                pos_manager->impl_->bond_map_[bond.expire_date].emplace_back(bond);
            }

            auto detail_func = [&](std::map<std::string, std::vector<Position>> &detail_map, Direction direction) {
                detail_map.clear();
                SQLite::Statement query_detail(db,
                                               "SELECT * FROM pos_detail WHERE account_id = ? and direction = ? ORDER BY ROWID");
                query_detail.bind(1, account_id_);
                query_detail.bind(2, std::string(1, direction));
                Position pos = {};
                while (query_detail.executeStep()) {
                    load_single_pos(query_detail, pos);
                    detail_map[get_symbol(pos.instrument_id, pos.exchange_id)].emplace_back(pos);
                }
            };
            detail_func(pos_manager->impl_->long_detail_map_, DirectionLong);
            detail_func(pos_manager->impl_->short_detail_map_, DirectionShort);

            pos_manager->impl_->frozen_map_.clear();
            SQLite::Statement query_frozen(db, "SELECT order_id, frozen_vol FROM pos_frozen WHERE account_id = ?");
            query_frozen.bind(1, account_id_);
            while (query_frozen.executeStep()) {
                pos_manager->impl_->frozen_map_[query_frozen.getColumn(0)] = query_frozen.getColumn(1);
            }

            db.exec("COMMIT");
        } catch (std::exception &e)
        {
            db.exec("ROLLBACK");
            SPDLOG_ERROR("failed to load position from db, {}", e.what());
        }
    }

    void PositionStorage::save_single_pos(SQLite::Database &db, const char *table, const kungfu::Position &pos)
    {
        std::string s = fmt::format("REPLACE INTO {}("
                                    "rcv_time, update_time, instrument_id, instrument_type, exchange_id, account_id, client_id, "
                                    "direction, volume, yesterday_volume, frozen_total, frozen_yesterday, last_price, open_price, "
                                    "cost_price, close_price, pre_close_price, settlement_price, pre_settlement_price, margin, "
                                    "position_pnl, close_pnl, realized_pnl, unrealized_pnl, open_date, expire_date"
                                    ") "
                                    "VALUES({},{},'{}','{}','{}','{}','{}','{}',{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},'{}','{}')",
                                    table, pos.rcv_time, pos.update_time, pos.instrument_id, pos.instrument_type,
                                    pos.exchange_id,
                                    pos.account_id, pos.client_id, pos.direction, pos.volume, pos.yesterday_volume,
                                    pos.frozen_total,
                                    pos.frozen_yesterday, pos.last_price, pos.open_price, pos.cost_price,
                                    pos.close_price,
                                    pos.pre_close_price, pos.settlement_price, pos.pre_settlement_price, pos.margin,
                                    pos.position_pnl,
                                    pos.close_pnl, pos.realized_pnl, pos.unrealized_pnl, pos.open_date,
                                    pos.expire_date);

        db.exec(s);
    }

    void PositionStorage::load_single_pos(SQLite::Statement &query, kungfu::Position &pos)
    {
        pos = {};
        pos.rcv_time = query.getColumn(0);
        pos.update_time = query.getColumn(1);
        strcpy(pos.instrument_id, query.getColumn(2));
        pos.instrument_type = query.getColumn(3)[0];
        strcpy(pos.exchange_id, query.getColumn(4));
        strcpy(pos.account_id, query.getColumn(5));
        strcpy(pos.client_id, query.getColumn(6));
        pos.direction = query.getColumn(7)[0];
        pos.volume = query.getColumn(8);
        pos.yesterday_volume = query.getColumn(9);
        pos.frozen_total = query.getColumn(10);
        pos.frozen_yesterday = query.getColumn(11);
        pos.last_price = query.getColumn(12);
        pos.open_price = query.getColumn(13);
        pos.cost_price = query.getColumn(14);
        pos.close_price = query.getColumn(15);
        pos.pre_close_price = query.getColumn(16);
        pos.settlement_price = query.getColumn(17);
        pos.pre_settlement_price = query.getColumn(18);
        pos.margin = query.getColumn(19);
        pos.position_pnl = query.getColumn(20);
        pos.close_pnl = query.getColumn(21);
        pos.realized_pnl = query.getColumn(22);
        pos.unrealized_pnl = query.getColumn(23);
        strcpy(pos.open_date, query.getColumn(24));
        strcpy(pos.expire_date, query.getColumn(25));
    }
}
