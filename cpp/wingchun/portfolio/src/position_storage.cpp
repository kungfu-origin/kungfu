//
// Created by PolarAir on 2019-04-16.
//

#include "position_storage.h"
#include "storage_common.h"
#include "util/include/business_helper.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    PositionStorage::PositionStorage(const char *account_id, const char *db_file) : account_id_(account_id), db_file_(db_file)
    {
        create_pos_tables(db_file);
    }

    void PositionStorage::save(int64_t last_update, const std::string &trading_day,
                               const std::map<std::string, kungfu::Position> &long_pos_map,
                               const std::map<std::string, kungfu::Position> &short_pos_map,
                               const std::map<std::string, std::vector<kungfu::Position>> &bond_map,
                               const std::map<std::string, std::vector<kungfu::Position>> &long_detail_map,
                               const std::map<std::string, std::vector<kungfu::Position>> &short_detail_map,
                               const std::map<int64_t, int64_t> &frozen_map)
    {
        SQLite::Database db(db_file_.c_str(), SQLite::OPEN_READWRITE);
        db.exec("BEGIN");
        try
        {
            db.exec(fmt::format("DELETE FROM position WHERE account_id = '{}' and instrument_type != '{}'", account_id_, InstrumentTypeFuture));
            db.exec(fmt::format("DELETE FROM bond_position WHERE account_id = '{}'", account_id_));
            db.exec(fmt::format("DELETE FROM pos_frozen WHERE account_id = '{}'", account_id_));

            for (const auto& iter : long_pos_map)
            {
                save_single_pos(db, "position", iter.second);
            }
            for (const auto& iter : short_pos_map)
            {
                save_single_pos(db, "position", iter.second);
            }

            for (const auto& iter : bond_map)
            {
                for (const auto& bond : iter.second)
                {
                    save_single_pos(db, "bond_position", bond);
                }
            }

            for (const auto& iter : long_detail_map)
            {
                for (const auto& detail : iter.second)
                {
                    save_single_pos(db, "pos_detail", detail);
                }
            }
            for (const auto& iter : short_detail_map)
            {
                for (const auto& detail : iter.second)
                {
                    save_single_pos(db, "pos_detail", detail);
                }
            }

            for (const auto& iter : frozen_map)
            {
                db.exec(fmt::format("INSERT INTO pos_frozen(order_id, account_id, frozen_vol) VALUES({}, '{}', {})",
                        iter.first, account_id_, iter.second));
            }

            save_meta_inner(db, last_update, trading_day);

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db.exec("ROLLBACK");
        }
    }

    void PositionStorage::load(int64_t &last_update, std::string &trading_day,
                               std::map<std::string, kungfu::Position> &long_pos_map,
                               std::map<std::string, kungfu::Position> &short_pos_map,
                               std::map<std::string, std::vector<kungfu::Position>> &bond_map,
                               std::map<std::string, std::vector<kungfu::Position>> &long_detail_map,
                               std::map<std::string, std::vector<kungfu::Position>> &short_detail_map,
                               std::map<int64_t, int64_t> &frozen_map)
    {
        SQLite::Database db(db_file_.c_str(), SQLite::OPEN_READONLY);
        db.exec("BEGIN");
        try
        {
            load_meta_inner(db, last_update, trading_day);

            auto pos_func = [&](std::map<std::string, Position>& pos_map, Direction direction) {
                pos_map.clear();
                SQLite::Statement query_pos(db, "SELECT * FROM position WHERE account_id = ? and direction = ?");
                query_pos.bind(1, account_id_);
                query_pos.bind(2, std::string(1, direction));
                Position pos = {};
                while (query_pos.executeStep())
                {
                    load_single_pos(query_pos, pos);
                    pos_map[get_symbol(pos.instrument_id, pos.exchange_id)] = pos;
                }
            };
            pos_func(long_pos_map, DirectionLong);
            pos_func(short_pos_map, DirectionShort);

            bond_map.clear();
            SQLite::Statement query_bond(db, "SELECT * FROM bond_position WHERE account_id = ?");
            query_bond.bind(1, account_id_);
            Position bond = {};
            while (query_bond.executeStep())
            {
                load_single_pos(query_bond, bond);
                bond_map[bond.expire_date].emplace_back(bond);
            }

            auto detail_func = [&](std::map<std::string, std::vector<Position>>& detail_map, Direction direction) {
                detail_map.clear();
                SQLite::Statement query_detail(db, "SELECT * FROM pos_detail WHERE account_id = ? and direction = ? ORDER BY ROWID");
                query_detail.bind(1, account_id_);
                query_detail.bind(2, std::string(1, direction));
                Position pos = {};
                while (query_detail.executeStep())
                {
                    load_single_pos(query_detail, pos);
                    detail_map[get_symbol(pos.instrument_id, pos.exchange_id)].emplace_back(pos);
                }
            };
            detail_func(long_detail_map, DirectionLong);
            detail_func(short_detail_map, DirectionShort);

            frozen_map.clear();
            SQLite::Statement query_frozen(db, "SELECT order_id, frozen_vol FROM pos_frozen WHERE account_id = ?");
            query_frozen.bind(1, account_id_);
            while (query_frozen.executeStep())
            {
                frozen_map[query_frozen.getColumn(0)] = query_frozen.getColumn(1);
            }

            db.exec("COMMIT");
        } catch (std::exception& e)
        {
            db.exec("ROLLBACK");
            SPDLOG_ERROR(e.what());
        }
    }

    void PositionStorage::save_single_pos(SQLite::Database &db, const char *table, const kungfu::Position &pos)
    {
        std::string s =fmt::format("REPLACE INTO {}("
                                   "rcv_time, update_time, instrument_id, instrument_type, exchange_id, account_id, client_id, "
                                   "direction, volume, yesterday_volume, frozen_total, frozen_yesterday, last_price, open_price, "
                                   "cost_price, close_price, pre_close_price, settlement_price, pre_settlement_price, margin, "
                                   "position_pnl, close_pnl, realized_pnl, unrealized_pnl, open_date, expire_date"
                                   ") "
                                   "VALUES({},{},'{}','{}','{}','{}','{}','{}',{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},'{}','{}')",
                                   table, pos.rcv_time, pos.update_time, pos.instrument_id, pos.instrument_type, pos.exchange_id,
                                   pos.account_id, pos.client_id, pos.direction, pos.volume, pos.yesterday_volume, pos.frozen_total,
                                   pos.frozen_yesterday, pos.last_price, pos.open_price, pos.cost_price, pos.close_price,
                                   pos.pre_close_price, pos.settlement_price, pos.pre_settlement_price, pos.margin, pos.position_pnl,
                                   pos.close_pnl, pos.realized_pnl, pos.unrealized_pnl, pos.open_date, pos.expire_date);

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

    void PositionStorage::save_meta_inner(SQLite::Database &db, int64_t last_update, const std::string &trading_day)
    {
        db.exec("DELETE FROM meta");
        db.exec(fmt::format("INSERT INTO meta(update_time, trading_day) VALUES({}, '{}')", last_update, trading_day));
    }

    void PositionStorage::load_meta_inner(SQLite::Database &db, int64_t &last_update, std::string &trading_day)
    {
        SQLite::Statement query(db, "SELECT * from meta");
        if (query.executeStep())
        {
            last_update = query.getColumn(0);
            trading_day = query.getColumn(1).getString();
        }
    }
}
