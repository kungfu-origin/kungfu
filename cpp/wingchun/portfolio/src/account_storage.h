//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_ACCOUNT_STORAGE_H
#define KUNGFU_ACCOUNT_STORAGE_H

#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    class AccountManager;
    class AccountStorage
    {
    public:
        explicit AccountStorage(const char* account_id);

        bool save(SQLite::Database& db, const AccountManager* acc_manager, bool save_meta);
        void load(SQLite::Database& db, AccountManager* acc_manager);

    protected:
        std::string account_id_;
    };
}

#endif //KUNGFU_ACCOUNT_STORAGE_H
