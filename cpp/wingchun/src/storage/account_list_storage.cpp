//
// Created by qlu on 2019/3/14.
//

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/storage/account_list_storage.h>

namespace kungfu
{
    namespace storage
    {
        AccountListStorage::AccountListStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        AccountListStorage::~AccountListStorage() {}
        
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

        void AccountListStorage::add_account(const std::string& client_id, const std::string& account_id, const std::string& source_id)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT INTO account_list (client_id, account_id, source) VALUES(?, ?, ?)");
                insert.bind(1, client_id);
                insert.bind(2, account_id);
                insert.bind(3, source_id);
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to add account({}, {}), exception: {}\n", source_id, account_id, e.what());
            }
        }

        std::vector<flying::SubPortfolioInfo> AccountListStorage::get_accounts()
        {
            std::vector<flying::SubPortfolioInfo> accounts;
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM account_list");
                while (query.executeStep())
                {
                    flying::SubPortfolioInfo account = {};
                    strcpy(account.account_id, query.getColumn(1));
                    strcpy(account.source_id, query.getColumn(2));
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
