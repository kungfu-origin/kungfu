//
// Created by PolarAir on 2019-04-14.
//

#include <kungfu/wingchun/commission/commission_storage.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/util/env.h>
#include <fmt/format.h>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <SQLiteCpp/Transaction.h>
#include <kungfu/wingchun/util/business_helper.h>

namespace kungfu
{
    std::map<std::string, std::vector<CommissionCallback>> CommissionStorage::s_cbs_;

    CommissionStorage::CommissionStorage(const char *account_id) : account_id_(account_id),
    db_(COMMISSION_DB_FILE(account_id)), hook_handle_(nullptr)
    {
        if (!boost::filesystem::exists(db_))
        {
            SPDLOG_WARN("commission db for account {} not found, using system default instead", account_id);
            if(!boost::filesystem::exists(ACCOUNT_FOLDER(account_id_)))
            {
                boost::filesystem::create_directories(ACCOUNT_FOLDER(account_id_));
            }
            boost::filesystem::copy_file(COMMISSION_DEFAULT_DB_FILE, db_);
        }

        create_table_if_not_exist();
        prepare_hook();
    }

    CommissionStorage::~CommissionStorage()
    {
        if (nullptr != hook_handle_)
        {
            sqlite3_close_v2(hook_handle_);
            hook_handle_ = nullptr;
        }
    }

    void CommissionStorage::load(kfj::InstrumentCommissionRate &stock_rate,
            std::map<std::string, kfj::InstrumentCommissionRate> &future_rates) const
    {
        stock_rate = {};
        future_rates.clear();
        future_rates[""] = {};
        try
        {
            SQLite::Database db(db_.c_str(), SQLite::OPEN_READONLY);

            SQLite::Statement query(db, "SELECT * FROM commission;");
            while (query.executeStep())
            {
                kfj::InstrumentCommissionRate rate = {};
                strcpy(rate.instrument_id, query.getColumn(0));
                strcpy(rate.exchange_id, query.getColumn(1));
                rate.instrument_type = query.getColumn(2)[0];
                strcpy(rate.account_id, query.getColumn(3));
                strcpy(rate.broker_id, query.getColumn(4));
                rate.mode = query.getColumn(5)[0];
                rate.open_ratio = query.getColumn(6);
                rate.close_ratio = query.getColumn(7);
                rate.close_today_ratio = query.getColumn(8);
                rate.min_commission = query.getColumn(9);

                strcpy(rate.account_id, account_id_.c_str());

                if (rate.instrument_type == InstrumentTypeStock || rate.instrument_type == InstrumentTypeBond)
                {
                    memcpy(&stock_rate, &rate, sizeof(kfj::InstrumentCommissionRate));
                }
                else
                {
                    strcpy(rate.exchange_id, get_exchange_id_from_future_instrument_id(rate.instrument_id).c_str());
                    future_rates[rate.instrument_id] = rate;
                }
            }
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void CommissionStorage::save(const kfj::InstrumentCommissionRate &rate) const
    {
        try
        {
            SQLite::Database db(db_.c_str(), SQLite::OPEN_READWRITE);
            single_replace(db, rate);

        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void CommissionStorage::save(const std::vector<kfj::InstrumentCommissionRate> &rates) const
    {
        try
        {
            SQLite::Database db(db_.c_str(), SQLite::OPEN_READWRITE);

            SQLite::Transaction transaction(db);
            for (const auto& rate : rates)
            {
                single_replace(db, rate);
            }
            transaction.commit();
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void CommissionStorage::save(const kfj::InstrumentCommissionRate &stock_rate,
            const std::map<std::string, kfj::InstrumentCommissionRate> &future_rates) const
    {
        try
        {
            SQLite::Database db(db_.c_str(), SQLite::OPEN_READWRITE);

            SQLite::Transaction transaction(db);
            single_replace(db, stock_rate);
            for (const auto& iter : future_rates)
            {
                single_replace(db, iter.second);
            }
            transaction.commit();
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void CommissionStorage::register_callback(CommissionCallback cb)
    {
        s_cbs_[account_id_].emplace_back(cb);
    }

    void CommissionStorage::create_table_if_not_exist() const
    {
        try
        {
            SQLite::Database db(db_.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

            SQLite::Transaction transaction(db);
            db.exec("CREATE TABLE IF NOT EXISTS commission("
                    "instrument_id CHAR(50), "
                    "exchange_id CHAR(50), "
                    "instrument_type CHAR(1), "
                    "account_id CHAR(50), "
                    "broker_id CHAR(50), "
                    "mode CHAR(1), "
                    "open_ratio DOUBLE, "
                    "close_ratio DOUBLE, "
                    "close_today_ratio DOUBLE, "
                    "min_commission DOUBLE)");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS commission_uid ON commission(instrument_id, exchange_id, instrument_type)");
            transaction. commit();
        } catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void CommissionStorage::prepare_hook()
    {
        if (sqlite3_open_v2(db_.c_str(), &hook_handle_, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK)
        {
            sqlite3_update_hook(hook_handle_, CommissionStorage::on_update, hook_handle_);
        }
        else
        {
            SPDLOG_ERROR("hook for commission of account {} failed", account_id_);
        }
    }

    void CommissionStorage::single_replace(SQLite::Database &db, const kfj::InstrumentCommissionRate &rate) const
    {
        SQLite::Statement replace(db, "REPLACE INTO commission(instrument_id, exchange_id, instrument_type, "
                                      "account_id, broker_id, mode, open_ratio, close_ratio, "
                                      "close_today_ratio, min_commission) "
                                      "VALUES(?,?,?,?,?,?,?,?,?,?);");
        replace.bind(1, rate.instrument_id);
        replace.bind(2, rate.exchange_id);
        replace.bind(3, std::string(1, rate.instrument_type));
        replace.bind(4, rate.account_id);
        replace.bind(5, rate.broker_id);
        replace.bind(6, rate.mode);
        replace.bind(7, rate.open_ratio);
        replace.bind(8, rate.close_ratio);
        replace.bind(9, rate.close_today_ratio);
        replace.bind(10, rate.min_commission);
        replace.exec();
    }

    void CommissionStorage::on_update(void *param, int action, const char *db, const char *table, sqlite3_int64 rowid)
    {
        if (action == SQLITE_INSERT || action == SQLITE_UPDATE)
        {
            auto hook_handle = (sqlite3*)param;
            auto func = [](void* p, int count, char** data, char** columns) {
                if (count == 10)
                {
                    kfj::InstrumentCommissionRate rate = {};
                    strcpy(rate.instrument_id, data[0]);
                    strcpy(rate.exchange_id, data[1]);
                    rate.instrument_type = data[2][0];
                    strcpy(rate.account_id, data[3]);
                    strcpy(rate.broker_id, data[4]);
                    rate.mode = data[5][0];
                    rate.open_ratio = atof(data[6]);
                    rate.close_ratio = atof(data[7]);
                    rate.close_today_ratio = atof(data[8]);
                    rate.min_commission = atof(data[9]);

                    auto p_cbs = (std::map<std::string, std::vector<CommissionCallback>>*)p;
                    if (p_cbs->find(rate.account_id) != p_cbs->end())
                    {
                        auto& cbs = p_cbs->at(rate.account_id);
                        for (auto& cb : cbs)
                        {
                            cb(rate);
                        }
                    }
                }
                return 0;
            };
            char** error = nullptr;
            if (SQLITE_OK != sqlite3_exec(hook_handle,
                    fmt::format("SELECT * FROM commission WHERE rowid = {};", rowid).c_str(),
                    func, &s_cbs_, error))
            {
                SPDLOG_ERROR(*error);
                sqlite3_free(error);
            }
        }
    }
}
