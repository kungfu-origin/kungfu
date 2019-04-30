//
// Created by qlu on 2019/3/8.
//

#ifndef KUNGFU_STRATEGY_LIST_STORAGE_H
#define KUNGFU_STRATEGY_LIST_STORAGE_H

#include <string>
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>

namespace kungfu
{
    class StrategyListStorage
    {
    public:
        StrategyListStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        void create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS strategy_list(strategy_name char(50) primary key);";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                std::cerr << __FILE__ << ":" << __LINE__ << " exception: " << e.what() << std::endl;
            }
        }

        void add_strategy(const std::string& strategy)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT OR IGNORE INTO strategy_list (strategy) VALUES(?)");
                insert.bind(1, strategy);
                insert.exec();
            }
            catch (std::exception &e)
            {
                std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << " strategy: " << strategy << " exception: " << e.what()<< std::endl;
            }
        }

        std::vector<std::string> get_strategies()
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
                std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << " exception: " << e.what()<< std::endl;
            }
            return strategies;
        }

    private:
        SQLite::Database db_;
    };
}

#endif //KUNGFU_STRATEGY_LIST_STORAGE_H
