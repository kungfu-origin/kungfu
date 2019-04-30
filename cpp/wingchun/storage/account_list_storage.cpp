//
// Created by qlu on 2019/3/14.
//

#include "account_list_storage.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace storage
    {
        AccountListStorage::AccountListStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        void AccountListStorage::create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS account_list(client_id char(50), account_id char(50) not null, source char(50) not null, type char(1), init_cash double, unique(source, account_id) on conflict ignore);";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to create table account_list, exception {}", e.what());
            }
        }

        void AccountListStorage::add_account(const StrategyUsedAccountInfo& info)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT INTO account_list (client_id, account_id, source, type, init_cash) VALUES(?, ?, ?, ?, ?)");
                insert.bind(1, info.client_id);
                insert.bind(2, info.account_id);
                insert.bind(3, info.source_id);
                insert.bind(4, info.type);
                insert.bind(5, info.init_cash);
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to add account({}, {}), exception: {}\n", info.source_id, info.account_id, e.what());
            }
        }

        std::vector<StrategyUsedAccountInfo> AccountListStorage::get_accounts()
        {
            std::vector<StrategyUsedAccountInfo> accounts;
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM account_list");
                while (query.executeStep())
                {
                    StrategyUsedAccountInfo account = {};
                    strcpy(account.client_id,query.getColumn(0));
                    strcpy(account.account_id, query.getColumn(1));
                    strcpy(account.source_id, query.getColumn(2));
                    account.type = query.getColumn(3).getString()[0];
                    account.init_cash = query.getColumn(4);
                    accounts.push_back(account);
                }
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to get accounts, exception: {}", e.what());
            }
            return accounts;
        }
    }
}
