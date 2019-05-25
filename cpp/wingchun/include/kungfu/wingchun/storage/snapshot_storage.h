//
// Created by qlu on 2019/3/14.
//

#ifndef KUNGFU_SNAPSHOT_STORAGE_H
#define KUNGFU_SNAPSHOT_STORAGE_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <kungfu/wingchun/oms_struct.h>

using namespace kungfu::flying;

namespace kungfu
{
    namespace storage
    {
        class SnapshotStorage
        {
        public:
            SnapshotStorage(const std::string& file_path, const std::string& table, bool daily, bool account);
            ~SnapshotStorage();

            void insert(const PortfolioInfo& portfolio_info);
            void insert(const AccountInfo& account_info);
        private:
            void create_table_if_not_exists(bool daily, bool account);
            SQLite::Database db_;
            std::string table_;
        };
    }
}
#endif //KUNGFU_SNAPSHOT_STORAGE_H
