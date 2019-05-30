//
// Created by PolarAir on 2019-02-27.
//

#ifndef KUNGFU_BASIC_INFO_HPP
#define KUNGFU_BASIC_INFO_HPP

#include <string>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/util/instrument.h>
#include <kungfu/wingchun/util/env.h>

using namespace kungfu::journal;

namespace kungfu
{
    FutureInstrumentStorage::FutureInstrumentStorage(const std::string& file_name) : db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
    {
        create_table_if_not_exist();
    }

    void FutureInstrumentStorage::create_table_if_not_exist()
    {
        try
        {
            std::string sql = "CREATE TABLE IF NOT EXISTS future_instrument(\n"
                              "                    instrument_id  CHAR(50),\n"
                              "                    exchange_id    CHAR(50),\n"
                              "                    instrument_type CHAR(1),\n"
                              "                    product_id     CHAR(50),\n"
                              "                    contract_multiplier   INTEGER,\n"
                              "                    price_tick     DOUBLE,\n"
                              "                    open_date      CHAR(50),\n"
                              "                    create_date    CHAR(50),\n"
                              "                    expire_date    CHAR(50),\n"
                              "                    delivery_year  INTEGER,\n"
                              "                    delivery_month INTEGER,\n"
                              "                    is_trading     INTEGER,\n"
                              "                    long_margin_ratio      DOUBLE,\n"
                              "                    short_margin_ratio     DOUBLE)\n";
            db_.exec(sql);
        }
        catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void FutureInstrumentStorage::set_future_instruments(const std::vector<FutureInstrument> &future_instruments)
    {
        db_.exec("BEGIN");
        try
        {
            db_.exec("DELETE FROM future_instrument");

            for (const auto& future_instrument : future_instruments)
            {
                SQLite::Statement insert(db_, "INSERT INTO future_instrument VALUES(?, ?, ?, ?, ?, ?, ?, ?, "
                                              "?, ?, ?, ?, ?, ?)");
                insert.bind(1, future_instrument.instrument_id);
                insert.bind(2, future_instrument.exchange_id);
                insert.bind(3, std::string(1, future_instrument.instrument_type));
                insert.bind(4, future_instrument.product_id);
                insert.bind(5, future_instrument.contract_multiplier);
                insert.bind(6, future_instrument.price_tick);
                insert.bind(7, future_instrument.open_date);
                insert.bind(8, future_instrument.create_date);
                insert.bind(9, future_instrument.expire_date);
                insert.bind(10, future_instrument.delivery_year);
                insert.bind(11, future_instrument.delivery_month);
                insert.bind(12, future_instrument.is_trading);
                insert.bind(13, future_instrument.long_margin_ratio);
                insert.bind(14, future_instrument.short_margin_ratio);
                insert.exec();
            }

            db_.exec("COMMIT");
        }
        catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
            db_.exec("ROLLBACK");
        }
    }

    void FutureInstrumentStorage::get_future_instrument(std::pair<std::string, std::string>& key, std::map<std::pair<std::string, std::string>, FutureInstrument>& future_instruments)
    {
        try
        {
            SQLite::Statement query(db_, "SELECT * FROM future_instrument WHERE instrument_id = ? and exchange_id = ?");
            query.bind(1, key.first);
            query.bind(2, key.second);
            if (query.executeStep())
            {
                FutureInstrument inst = {};
                strcpy(inst.instrument_id, query.getColumn(0));
                strcpy(inst.exchange_id, query.getColumn(1));
                inst.instrument_type = query.getColumn(2)[0];
                strcpy(inst.product_id, query.getColumn(3));
                inst.contract_multiplier = query.getColumn(4);
                inst.price_tick = query.getColumn(5);
                strcpy(inst.open_date, query.getColumn(6));
                strcpy(inst.create_date, query.getColumn(7));
                strcpy(inst.expire_date, query.getColumn(8));
                inst.delivery_year = query.getColumn(9);
                inst.delivery_month = query.getColumn(10);
                inst.is_trading = (int)query.getColumn(11);
                inst.long_margin_ratio = query.getColumn(12);
                inst.short_margin_ratio = query.getColumn(13);
                future_instruments[key] = inst;
            }
        }
        catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    void FutureInstrumentStorage::get_future_instruments(std::map<std::pair<std::string, std::string>, FutureInstrument>& future_instruments)
    {
        try
        {
            SQLite::Statement query(db_, "SELECT * FROM future_instrument");
            while (query.executeStep())
            {
                std::pair<std::string, std::string> key = {(const char*)query.getColumn(0), (const char*)query.getColumn(1)};
                FutureInstrument inst = {};
                strcpy(inst.instrument_id, query.getColumn(0));
                strcpy(inst.exchange_id, query.getColumn(1));
                inst.instrument_type = query.getColumn(2)[0];
                strcpy(inst.product_id, query.getColumn(3));
                inst.contract_multiplier = query.getColumn(4);
                inst.price_tick = query.getColumn(5);
                strcpy(inst.open_date, query.getColumn(6));
                strcpy(inst.create_date, query.getColumn(7));
                strcpy(inst.expire_date, query.getColumn(8));
                inst.delivery_year = query.getColumn(9);
                inst.delivery_month = query.getColumn(10);
                inst.is_trading = (int)query.getColumn(11);
                inst.long_margin_ratio = query.getColumn(12);
                inst.short_margin_ratio = query.getColumn(13);
                future_instruments[key] = inst;
            }
        }
        catch (std::exception& e)
        {
            SPDLOG_ERROR(e.what());
        }
    }

    std::shared_ptr<InstrumentManager> InstrumentManager::instrument_manager_ = nullptr;

    InstrumentManager::InstrumentManager()
    {
        memset(&default_future_instrument_, 0, sizeof(default_future_instrument_));
        default_future_instrument_.contract_multiplier = 10;
        default_future_instrument_.price_tick = 1.0;
        default_future_instrument_.is_trading = true;
        default_future_instrument_.long_margin_ratio = 0.2;
        default_future_instrument_.short_margin_ratio = 0.2;
        reload_from_db();
    }

    void InstrumentManager::reload_from_db()
    {
        future_instruments_.clear();
        FutureInstrumentStorage storage_(fmt::format(FUTURE_INSTRUMENT_DB_FILE_FORMAT, get_base_dir()));
        storage_.get_future_instruments(future_instruments_);
    }

    std::shared_ptr<InstrumentManager> InstrumentManager::get_instrument_manager()
    {
        if (instrument_manager_ == nullptr)
        {
            instrument_manager_ = std::shared_ptr<InstrumentManager>(new InstrumentManager());
        }
        return instrument_manager_;
    }

    const FutureInstrument* InstrumentManager::get_future_instrument(const char* instrument_id, const char* exchange_id)
    {
        std::pair<std::string, std::string> key = {std::string(instrument_id), std::string(exchange_id)};
        auto search = future_instruments_.find(key);
        if (search != future_instruments_.end())
        {
            return &(search->second);
        }
        else
        {
            FutureInstrumentStorage storage_(fmt::format(FUTURE_INSTRUMENT_DB_FILE_FORMAT, get_base_dir()));
            storage_.get_future_instrument(key, future_instruments_);
            auto search = future_instruments_.find(key);
            if (search != future_instruments_.end())
            {
                return &(search->second);
            }
        }
        return &default_future_instrument_;
    }

    int InstrumentManager::get_future_multiplier(const char* instrument_id, const char* exchange_id)
    {
        const auto* ins = get_future_instrument(instrument_id, exchange_id);
        if (nullptr != ins)
        {
            return ins->contract_multiplier;
        }
        return default_future_instrument_.contract_multiplier;
    }

    double InstrumentManager::get_future_margin_ratio(const char* instrument_id, const char* exchange_id, kungfu::Direction dir)
    {
        const auto* ins = get_future_instrument(instrument_id, exchange_id);
        if (nullptr != ins)
        {
            return dir == DirectionLong ? ins->long_margin_ratio : ins->short_margin_ratio;
        }
        return dir == DirectionLong ? default_future_instrument_.long_margin_ratio : default_future_instrument_.short_margin_ratio;
    }

    std::vector<const FutureInstrument*> InstrumentManager::get_future_instruments()
    {
        FutureInstrumentStorage storage_(fmt::format(FUTURE_INSTRUMENT_DB_FILE_FORMAT, get_base_dir()));
        storage_.get_future_instruments(future_instruments_);
        std::vector<const FutureInstrument*> result;
        for (const auto& iter : future_instruments_)
        {
            result.push_back(&(iter.second));
        }
        return result;
    }
}

#endif // KUNGFU_BASIC_INFO_HPP
