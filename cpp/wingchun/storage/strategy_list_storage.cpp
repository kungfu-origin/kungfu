//
// Created by qlu on 2019/3/14.
//

#include "strategy_list_storage.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace storage
    {
        StrategyListStorage::StrategyListStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        void StrategyListStorage::create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS strategy_list(strategy_name char(50) primary key);";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to create table strategy_list , exception: {}", e.what());
            }
        }

        void StrategyListStorage::add_strategy(const std::string& strategy)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT OR IGNORE INTO strategy_list (strategy) VALUES(?)");
                insert.bind(1, strategy);
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to add strategy {}, exception: {}", strategy.c_str(), e.what());
            }
        }

        std::vector<std::string> StrategyListStorage::get_strategies()
        {
            std::vector<std::string> strategies;
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM strategy_list");
                while (query.executeStep())
                {
                    std::string strategy = query.getColumn(0);
                    strategies.push_back(strategy);
                }
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to get strategies, exception: {}", e.what());
            }
            return strategies;
        }
    }
}
