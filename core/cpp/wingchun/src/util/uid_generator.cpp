//
// Created by qlu on 2019/1/18.
//

#include <kungfu/wingchun/util/uid_generator.h>
#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>

namespace kungfu
{
    UidWorkerStorage* UidWorkerStorage::instance_ = nullptr;
    UidWorkerStorage* UidWorkerStorage::get_instance(const std::string& file_name)
    {
        if (instance_ == nullptr)
        {
            UidWorkerStorage::instance_ = new UidWorkerStorage(file_name);
        }
        return UidWorkerStorage::instance_;
    }

    UidWorkerStorage::UidWorkerStorage(const std::string& file_name): file_name_(file_name)
    {
        create_table_if_not_exist();
    }

    void UidWorkerStorage::create_table_if_not_exist()
    {
        try
        {
            SQLite::Database db(file_name_.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            std::string sql = "CREATE TABLE IF NOT EXISTS uid_worker("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                    "name        CHAR(50) UNIQUE);";
            db.exec(sql);
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }

    void UidWorkerStorage::add_uid_worker(const std::string& name)
    {
        try
        {
            SQLite::Database db(file_name_.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement insert(db, "INSERT OR IGNORE INTO uid_worker VALUES (NULL, ?)");
            insert.bind(1, name);
            insert.exec();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }

    int UidWorkerStorage::get_uid_worker_id(const std::string &name)
    {
        try
        {
            SQLite::Database db(file_name_.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db, "SELECT id FROM uid_worker WHERE name == ?");
            query.bind(1, name);
            if (query.executeStep())
            {
                int id = query.getColumn(0);
                return id;
            }
            else
            {
                return -1;
            }
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
            return -1;
        }

    }
}