//
// Created by qlu on 2019/3/14.
//

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/storage/source_list_storage.h>

namespace kungfu
{
    namespace storage
    {
        SourceListStorage::SourceListStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }
        SourceListStorage::~SourceListStorage() {};
        void SourceListStorage::create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS source_list(source char(50) primary key);";
                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to create table source_list, exception: {}", e.what());
            }
        }

        void SourceListStorage::add_source(const std::string& source)
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT OR IGNORE INTO source_list (source) VALUES(?)");
                insert.bind(1, source);
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to insert source {}, exception: {}", source.c_str(), e.what());
            }
        }

        std::vector<std::string> SourceListStorage::get_sources()
        {
            std::vector<std::string> sources;
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM source_list");
                while (query.executeStep())
                {
                    std::string source = query.getColumn(0);
                    sources.push_back(source);
                }
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR("failed to get sources, exception: {}", e.what());
            }
            return sources;
        }
    }
}
