//
// Created by qlu on 2019/3/14.
//

#ifndef KUNGFU_STRATEGY_STORAGE_H
#define KUNGFU_STRATEGY_STORAGE_H

#include <string>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    namespace storage
    {
        class StrategyListStorage
        {
        public:
            StrategyListStorage(const std::string& file_name);

            void create_table_if_not_exist();
            void add_strategy(const std::string& strategy);
            std::vector<std::string> get_strategies();

        private:
            SQLite::Database db_;
        };
    }
}

#endif //KUNGFU_STRATEGY_STORAGE_H
