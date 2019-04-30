//
// Created by PolarAir on 2019-04-17.
//

#ifndef KUNGFU_ACCOUNT_STORAGE_H
#define KUNGFU_ACCOUNT_STORAGE_H

#include "oms_struct.h"
#include <map>

namespace kungfu
{
    class AccountStorage
    {
    public:
        AccountStorage(const char* account_id, const char* db_file);

        void save(int64_t last_update, const std::string& trading_day, const AccountInfo& account,
                const std::map<std::string, double>& bond_map, const std::map<int64_t, double>& frozen_map);
        void save_meta(int64_t last_update, const std::string& trading_day);

        void load(int64_t& last_update, std::string& trading_day, AccountInfo& account,
                std::map<std::string, double>& bond_map, std::map<int64_t, double>& frozen_map);

    protected:
        std::string account_id_;
        std::string db_file_;
    };
}

#endif //KUNGFU_ACCOUNT_STORAGE_H
