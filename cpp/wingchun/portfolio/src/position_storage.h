//
// Created by PolarAir on 2019-04-16.
//

#ifndef KUNGFU_POSITION_STORAGE_H
#define KUNGFU_POSITION_STORAGE_H

#include "oms_struct.h"
#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    class PositionManager;
    class PositionStorage
    {
    public:
        explicit PositionStorage(const char* account_id);

        bool save(SQLite::Database& db, const PositionManager* pos_manager, bool save_meta);
        void load(SQLite::Database& db, PositionManager* pos_manager);

    protected:
        static void save_single_pos(SQLite::Database& db, const char* table, const Position& pos);
        static void load_single_pos(SQLite::Statement& query, Position& pos);

    protected:
        std::string account_id_;
    };
}

#endif //KUNGFU_POSITION_STORAGE_H