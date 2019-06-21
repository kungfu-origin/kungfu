//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_STORAGE_H
#define WINGCHUN_STORAGE_H

#include <string>
#include <map>
#include <functional>
#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include <kungfu/common.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace storage
        {

            typedef std::function<void(const msg::data::InstrumentCommissionRate &)> CommissionCallback;

            class CommissionStorage
            {
            public:
                explicit CommissionStorage(const std::string& db, const std::string &file_path);

                ~CommissionStorage();

                void load(msg::data::InstrumentCommissionRate &stock_rate, std::map<std::string, msg::data::InstrumentCommissionRate> &future_rates) const;

                void save(const msg::data::InstrumentCommissionRate &rate) const;

                void save(const std::vector<msg::data::InstrumentCommissionRate> &rates) const;

                void
                save(const msg::data::InstrumentCommissionRate &stock_rate, const std::map<std::string, msg::data::InstrumentCommissionRate> &future_rates) const;

                void register_callback(CommissionCallback cb);

            private:
                void prepare_hook();

                void single_replace(SQLite::Database &db, const msg::data::InstrumentCommissionRate &rate) const;

                static void on_update(void *param, int action, const char *db, const char *table, sqlite3_int64 rowid);

                std::string account_id_;
                std::string db_;
                sqlite3 *hook_handle_;
                static std::map<std::string, std::vector<CommissionCallback>> s_cbs_;
            };

            class DatabaseHolder
            {
            public:
                explicit DatabaseHolder(const std::string &file_path) : db_(file_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {};

                virtual ~DatabaseHolder() = default;

            protected:
                SQLite::Database db_;
            };

            class UidWorkerStorage : DatabaseHolder
            {
            public:
                UidWorkerStorage(const std::string &file_path);

                void add_uid_worker(const std::string& name);

                int get_uid_worker_id(const std::string& name);
            };

            class CalendarStorage : DatabaseHolder
            {
            public:
                CalendarStorage(const std::string &file_path);

                void get_trading_days(int from_year, int to_year, std::vector<int> &trading_days, const std::string& region);
            };

            class OrderStorage : DatabaseHolder
            {
            public:
                OrderStorage(const std::string &file_path);

                void add_order(uint64_t order_id, const msg::data::Order &order);

                const msg::data::Order get_order(uint64_t order_id);
            };

            DECLARE_PTR(OrderStorage)

            class TradeStorage : DatabaseHolder
            {
            public:
                TradeStorage(const std::string &file_path);

                long long add_trade(const msg::data::Trade &trade);
            };

            DECLARE_PTR(TradeStorage)

            class AccountListStorage : DatabaseHolder
            {
            public:
                AccountListStorage(const std::string &file_name);

                void add_account(const std::string &client_id, const std::string &account_id, const std::string &source_id);

                std::vector<msg::data::SubPortfolioInfo> get_accounts();
            };

            DECLARE_PTR(AccountListStorage)

            class SourceListStorage : DatabaseHolder
            {
            public:
                SourceListStorage(const std::string &file_name);

                void add_source(const std::string &source);

                std::vector<std::string> get_sources();
            };

            DECLARE_PTR(SourceListStorage)

            class SnapshotStorage : DatabaseHolder
            {
            public:
                SnapshotStorage(const std::string &file_path, const std::string &table, bool daily, bool account);

                void insert(const msg::data::PortfolioInfo &portfolio_info);

                void insert(const msg::data::AccountInfo &account_info);

            private:
                std::string table_;
            };

            DECLARE_PTR(SnapshotStorage)
        }
    }
}

#endif //WINGCHUN_STORAGE_H
