//
// Created by PolarAir on 2019-04-18.
//

#ifndef KUNGFU_PORTFOLIO_STORAGE_H
#define KUNGFU_PORTFOLIO_STORAGE_H

#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    class PortfolioManager;
    class PortfolioStorage
    {
    public:
        explicit PortfolioStorage(const char* name);

        bool save(SQLite::Database& db, const PortfolioManager* pnl_manager);
        void load(SQLite::Database& db, PortfolioManager* pnl_manager);

    protected:
        std::string name_;
    };
}

#endif //KUNGFU_PORTFOLIO_STORAGE_H
