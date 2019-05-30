//
// Created by qlu on 2019/3/14.
//

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/storage/snapshot_storage.h>

namespace kungfu
{
    namespace storage
    {
        SnapshotStorage::SnapshotStorage(const std::string &file_path, const std::string& table, bool daily, bool account): db_(file_path.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE), table_(table)
        {
            create_table_if_not_exists(daily, account);
        }

        SnapshotStorage::~SnapshotStorage() {}
        
        void SnapshotStorage::create_table_if_not_exists(bool daily, bool account)
        {
            static std::string columns =
                            "id                    INTEGER primary key autoincrement,"
                            "update_time           INTEGER,"
                            "trading_day           char(50),"
                            "account_id            char(50),"
                            "type                  char,"
                            "broker_id             char(50),"
                            "source_id             char(50),"
                            "initial_equity        double,"
                            "static_equity         double,"
                            "dynamic_equity        double,"
                            "accumulated_pnl       double,"
                            "accumulated_pnl_ratio double,"
                            "intraday_pnl          double,"
                            "intraday_pnl_ratio    double,"
                            "avail                 double,"
                            "market_value          double,"
                            "margin                double,"
                            "accumulated_fee       double,"
                            "intraday_fee          double,"
                            "frozen_cash           double,"
                            "frozen_margin         double,"
                            "frozen_fee            double,"
                            "position_pnl          double,"
                            "close_pnl             double"
            ;
            static std::string daily_uid_columns =  account ? "account_id, trading_day" : "trading_day";
            static std::string minutely_uid_columns = account ? "account_id, update_time" : "update_time";

            std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS {}({});", table_, columns);
            std::string uid = fmt::format("CREATE UNIQUE INDEX IF NOT EXISTS {}_uid ON {}({});", table_, table_, daily ? daily_uid_columns : minutely_uid_columns);
            try
            {
                db_.exec("BEGIN");

                db_.exec(sql);
                db_.exec(uid);

                db_.exec("COMMIT");
            }
            catch (std::exception& e)
            {
                db_.exec("ROLLBACK");
                SPDLOG_ERROR("failed to create table {}, \nsql: {}, \nuid: {}, \nexception: {}", table_.c_str(), sql.c_str(), uid.c_str(), e.what());
            }
        }

        void SnapshotStorage::insert(const AccountInfo &account_info)
        {
            SQLite::Statement insert(db_, "REPLACE INTO " + table_ + "(update_time, trading_day, account_id, type, "
                    "broker_id, source_id, initial_equity, static_equity, dynamic_equity, "
                    "accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio, "
                    "avail, market_value, margin, accumulated_fee, intraday_fee, "
                    "frozen_cash, frozen_margin, frozen_fee, position_pnl, close_pnl) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            insert.bind(1, account_info.update_time);
            insert.bind(2, account_info.trading_day);
            insert.bind(3, account_info.account_id);
            insert.bind(4, std::string(1, account_info.account_type));
            insert.bind(5, account_info.broker_id);
            insert.bind(6, account_info.source_id);
            insert.bind(7, account_info.initial_equity);
            insert.bind(8, account_info.static_equity);
            insert.bind(9, account_info.dynamic_equity);
            insert.bind(10, account_info.accumulated_pnl);
            insert.bind(11, account_info.accumulated_pnl_ratio);
            insert.bind(12, account_info.intraday_pnl);
            insert.bind(13, account_info.intraday_pnl_ratio);
            insert.bind(14, account_info.avail);
            insert.bind(15, account_info.market_value);
            insert.bind(16, account_info.margin);
            insert.bind(17, account_info.accumulated_fee);
            insert.bind(18, account_info.intraday_fee);
            insert.bind(19, account_info.frozen_cash);
            insert.bind(20, account_info.frozen_margin);
            insert.bind(21, account_info.frozen_fee);
            insert.bind(22, account_info.position_pnl);
            insert.bind(23, account_info.close_pnl);

            try
            {
                insert.exec();
            }
            catch (std::exception& e)
            {
                SPDLOG_ERROR("failed to insert into table {}, exception: {}", table_.c_str(), e.what());
            }
        }

        void SnapshotStorage::insert(const PortfolioInfo& portfolio_info)
        {
            try
            {
                SQLite::Statement insert(db_, "REPLACE INTO " + table_ + "(update_time, trading_day, initial_equity, static_equity, "
                        "dynamic_equity, accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio) "
                        "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
                insert.bind(1, portfolio_info.update_time);
                insert.bind(2, portfolio_info.trading_day);
                insert.bind(3, portfolio_info.initial_equity);
                insert.bind(4, portfolio_info.static_equity);
                insert.bind(5, portfolio_info.dynamic_equity);
                insert.bind(6, portfolio_info.accumulated_pnl);
                insert.bind(7, portfolio_info.accumulated_pnl_ratio);
                insert.bind(8, portfolio_info.intraday_pnl);
                insert.bind(9, portfolio_info.intraday_pnl_ratio);
                insert.exec();
            }
            catch (std::exception& e)
            {
                SPDLOG_ERROR("failed to insert into table {}, exception: {}", table_.c_str(), e.what());
            }
        }
    }
}