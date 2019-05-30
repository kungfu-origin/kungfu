//
// Created by qlu on 2019/3/14.
//

#ifndef KUNGFU_TRADE_STORAGE_H
#define KUNGFU_TRADE_STORAGE_H

#include <string>
#include <SQLiteCpp/SQLiteCpp.h>

#include <kungfu/wingchun/oms_struct.h>

using namespace kungfu::journal;

namespace kungfu
{
    namespace storage
    {
        class TradeStorage
        {
        public:
            TradeStorage(const std::string& file_path);
            ~TradeStorage();
            void create_table_if_not_exist();
            long long add_trade(const Trade &trade);
        private:
            SQLite::Database db_;
        };
    }
}
#endif //KUNGFU_TRADE_STORAGE_H
