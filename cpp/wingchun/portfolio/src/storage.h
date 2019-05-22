//
// Created by PolarAir on 2019-04-24.
//

#ifndef KUNGFU_STORAGE_COMMON_H
#define KUNGFU_STORAGE_COMMON_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace portfolio_util
    {
        void init_db(const char* db_file);

        void save_meta_inner(SQLite::Database& db, int64_t last_update, const std::string& trading_day);
        void load_meta_inner(SQLite::Database& db, int64_t& last_update, std::string& trading_day);

        void create_pos_tables(SQLite::Database& db);
        void create_acc_tables(SQLite::Database& db);
        void create_pnl_tables(SQLite::Database& db);
        void create_meta_table(SQLite::Database& db);
    }
}

#endif //KUNGFU_STORAGE_COMMON_H
