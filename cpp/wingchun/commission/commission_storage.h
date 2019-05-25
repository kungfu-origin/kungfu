//
// Created by PolarAir on 2019-04-14.
//

#ifndef KUNGFU_COMMISSION_STORAGE_H
#define KUNGFU_COMMISSION_STORAGE_H

#include <kungfu/wingchun/oms_struct.h>
#include <map>
#include <functional>
#include <SQLiteCpp/Database.h>
#include <sqlite3.h>

namespace kfj = kungfu::journal;

namespace kungfu
{
    typedef std::function<void(const kfj::InstrumentCommissionRate&)> CommissionCallback;
    class CommissionStorage
    {
    public:
        explicit CommissionStorage(const char* db);
        ~CommissionStorage();

        void load(kfj::InstrumentCommissionRate& stock_rate, std::map<std::string, kfj::InstrumentCommissionRate>& future_rates) const;
        void save(const kfj::InstrumentCommissionRate& rate) const;
        void save(const std::vector<kfj::InstrumentCommissionRate>& rates) const;
        void save(const kfj::InstrumentCommissionRate& stock_rate, const std::map<std::string, kfj::InstrumentCommissionRate>& future_rates) const;

        void register_callback(CommissionCallback cb);

    private:
        void create_table_if_not_exist() const;
        void prepare_hook();
        void single_replace(SQLite::Database& db, const kfj::InstrumentCommissionRate& rate) const;
        static void on_update(void* param, int action, const char* db, const char* table, sqlite3_int64 rowid);

        std::string account_id_;
        std::string db_;
        sqlite3* hook_handle_;
        static std::map<std::string, std::vector<CommissionCallback>> s_cbs_;
    };
}

#endif //KUNGFU_COMMISSION_STORAGE_H
