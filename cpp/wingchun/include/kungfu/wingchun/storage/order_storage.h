//
// Created by qlu on 2019/3/14.
//

#ifndef KUNGFU_ORDER_STORAGE_H
#define KUNGFU_ORDER_STORAGE_H

#include <string>
#include <SQLiteCpp/SQLiteCpp.h>
#include <kungfu/wingchun/oms_struct.h>

namespace kungfu
{
    namespace storage
    {
        class OrderStorage
        {
        public:
            OrderStorage(const std::string& file_path);
            ~OrderStorage();
            void create_table_if_not_exist();
            void add_order(uint64_t order_id, const journal::Order &order);
            const journal::Order get_order(uint64_t order_id);
        private:
            SQLite::Database db_;
        };
    }
}
#endif //KUNGFU_ORDER_STORAGE_H
