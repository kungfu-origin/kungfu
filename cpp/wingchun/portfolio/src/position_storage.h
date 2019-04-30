//
// Created by PolarAir on 2019-04-16.
//

#ifndef KUNGFU_POSITION_STORAGE_H
#define KUNGFU_POSITION_STORAGE_H

#include "oms_struct.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <map>
#include <vector>

namespace kungfu
{
    class PositionStorage
    {
    public:
        PositionStorage(const char* account_id, const char* db_file);

        void save(int64_t last_update,
                  const std::string& trading_day,
                  const std::map<std::string, Position>& long_pos_map,
                  const std::map<std::string, Position>& short_pos_map,
                  const std::map<std::string, std::vector<Position>>& bond_map,
                  const std::map<std::string, std::vector<Position>>& long_detail_map,
                  const std::map<std::string, std::vector<Position>>& short_detail_map,
                  const std::map<int64_t, int64_t>& frozen_map);

        void load(int64_t& last_update,
                  std::string& trading_day,
                  std::map<std::string, Position>& long_pos_map,
                  std::map<std::string, Position>& short_pos_map,
                  std::map<std::string, std::vector<Position>>& bond_map,
                  std::map<std::string, std::vector<Position>>& long_detail_map,
                  std::map<std::string, std::vector<Position>>& short_detail_map,
                  std::map<int64_t, int64_t>& frozen_map);

    protected:
        static void save_single_pos(SQLite::Database& db, const char* table, const Position& pos);
        static void load_single_pos(SQLite::Statement& query, Position& pos);
        static void save_meta_inner(SQLite::Database& db, int64_t last_update, const std::string& trading_day);
        static void load_meta_inner(SQLite::Database& db, int64_t& last_update, std::string& trading_day);

    protected:
        std::string account_id_;
        std::string db_file_;
    };
}

#endif //KUNGFU_POSITION_STORAGE_H
