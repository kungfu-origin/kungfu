//
// Created by PolarAir on 2019-04-18.
//

#ifndef KUNGFU_PORTFOLIO_STORAGE_H
#define KUNGFU_PORTFOLIO_STORAGE_H

#include "oms_struct.h"
#include "../include/account_manager.h"
#include <map>

namespace kungfu
{
    class PortfolioStorage
    {
    public:
        explicit PortfolioStorage(const char* db_file);

        void save(int64_t last_update, const std::string& trading_day, const PortfolioInfo& pnl);
        void save_meta(int64_t last_update, const std::string& trading_day);

        void load(int64_t& last_update, std::string& trading_day, PortfolioInfo& pnl, std::map<std::string, AccountManagerPtr>& stock_accounts);

    protected:
        std::string db_file_;
    };
}

#endif //KUNGFU_PORTFOLIO_STORAGE_H
