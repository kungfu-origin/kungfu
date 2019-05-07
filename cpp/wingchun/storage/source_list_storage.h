//
// Created by qlu on 2019/3/8.
//

#ifndef KUNGFU_SOURCE_LIST_STORAGE_H
#define KUNGFU_SOURCE_LIST_STORAGE_H

#include <string>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    namespace storage
    {
        class SourceListStorage
        {
        public:
            SourceListStorage(const std::string& file_name);
            ~SourceListStorage();
            void create_table_if_not_exist();
            void add_source(const std::string& source);
            std::vector<std::string> get_sources();

        private:
            SQLite::Database db_;
        };

    }
}
#endif //KUNGFU_SOURCE_LIST_STORAGE_H
