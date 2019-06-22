//
// Created by Keren Dong on 2019-06-20.
//

#include <set>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/storage/storage.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace storage
        {
            UidWorkerStorage::UidWorkerStorage(const std::string &file_path) : DatabaseHolder(file_path)
            {
                    std::string sql = "CREATE TABLE IF NOT EXISTS uid_worker("
                                      "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                                      "name        CHAR(50) UNIQUE);";
                    db_.exec(sql);
            }

            void UidWorkerStorage::add_uid_worker(const std::string &name)
            {
                    SQLite::Statement insert(db_, "INSERT OR IGNORE INTO uid_worker VALUES (NULL, ?)");
                    insert.bind(1, name);
                    insert.exec();
            }

            int UidWorkerStorage::get_uid_worker_id(const std::string &name)
            {
                    SQLite::Statement query(db_, "SELECT id FROM uid_worker WHERE name == ?");
                    query.bind(1, name);
                    if (query.executeStep())
                    {
                        int id = query.getColumn(0);
                        return id;
                    } else
                    {
                        return -1;
                    }
            }

            CalendarStorage::CalendarStorage(const std::string &file_path) : DatabaseHolder(file_path)
            {
                try
                {
                    std::string sql = "CREATE TABLE IF NOT EXISTS holidays(\n"
                                      "                    region       CHAR(50),\n"
                                      "                    holiday      INTEGER)\n";
                    db_.exec(sql);
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR(e.what());
                }
            }

            void CalendarStorage::get_trading_days(int from_year, int to_year, std::vector<int> &trading_days, const std::string& region)
            {
                try
                {
                    std::set<int> holidays{};
                    SQLite::Statement query(db_, "SELECT holiday FROM holidays WHERE region = ?");
                    query.bind(1, region);
                    if (query.executeStep())
                    {
                        holidays.insert(query.getColumn(0));
                    }
                    int start = 0;
                    for (int year = from_year; year <= to_year; year++)
                    {
                        std::vector<int> months = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
                        {
                            months[2] = 29;
                        }
                        for (int month = 1; month <= 12; month++)
                        {
                            for (int day = 1; day <= months[month]; day++)
                            {
                                if ((start + day) % 7 >= 1 && (start + day) % 7 <= 5)
                                {
                                    int current_day = year * 10000 + month * 100 + day;
                                    auto search = holidays.find(current_day);
                                    if (search == holidays.end())
                                    {
                                        trading_days.push_back(current_day);
                                    }
                                }
                            }
                            start += months[month];
                        }
                    }

                } catch (std::exception &e)
                {
                    SPDLOG_ERROR(e.what());
                }
            }

            OrderStorage::OrderStorage(const std::string &file_path) : DatabaseHolder(file_path)
            {
                try
                {
                    std::string sql = "CREATE TABLE IF NOT EXISTS orders(\n"
                                      "                    order_id         CHAR(50) PRIMARY KEY NOT NULL,\n"
                                      "                    rcv_time         INTEGER,\n"
                                      "                    insert_time      INTEGER,\n"
                                      "                    update_time      INTEGER,\n"
                                      "                    trading_day      CHAR(50),\n"
                                      "                    instrument_id    CHAR(50),\n"
                                      "                    exchange_id      CHAR(50),\n"
                                      "                    account_id       CHAR(50),\n"
                                      "                    client_id        CHAR(50),\n"
                                      "                    instrument_type  CHAR(1),\n"
                                      "                    limit_price      DOUBLE,\n"
                                      "                    frozen_price     DOUBLE,\n"
                                      "                    volume           INTEGER,\n"
                                      "                    volume_traded    INTEGER,\n"
                                      "                    volume_left      INTEGER,\n"
                                      "                    tax              DOUBLE,\n"
                                      "                    commission       DOUBLE,\n"
                                      "                    status           CHAR(1),\n"
                                      "                    error_id         INTEGER,\n"
                                      "                    error_msg        TEXT,\n"
                                      "                    side             CHAR(1),\n"
                                      "                    offset           CHAR(1),\n"
                                      "                    price_type       CHAR(1),\n"
                                      "                    volume_condition CHAR(1),\n"
                                      "                    time_condition   CHAR(1),\n"
                                      "                    parent_id        CHAR(50));";
                    db_.exec(sql);
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to create table {}, exception: {}", "orders", e.what());
                }
            }

            void OrderStorage::add_order(uint64_t order_id, const msg::data::Order &order)
            {
                try
                {
                    SQLite::Statement insert(db_,
                                             "INSERT OR REPLACE INTO orders VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                    insert.bind(1, std::to_string(order.order_id));
                    insert.bind(2, (long long) order.rcv_time);
                    insert.bind(3, order.insert_time);
                    insert.bind(4, order.update_time);
                    insert.bind(5, order.trading_day);
                    insert.bind(6, order.instrument_id);
                    insert.bind(7, order.exchange_id);
                    insert.bind(8, order.account_id);
                    insert.bind(9, order.client_id);
                    insert.bind(10, std::string(1, order.instrument_type));
                    insert.bind(11, order.limit_price);
                    insert.bind(12, order.frozen_price);
                    insert.bind(13, order.volume);
                    insert.bind(14, order.volume_traded);
                    insert.bind(15, order.volume_left);
                    insert.bind(16, order.tax);
                    insert.bind(17, order.commission);
                    insert.bind(18, std::string(1, order.status));
                    insert.bind(19, order.error_id);
                    insert.bind(20, order.error_msg);
                    insert.bind(21, std::string(1, order.side));
                    insert.bind(22, std::string(1, order.offset));
                    insert.bind(23, std::string(1, order.price_type));
                    insert.bind(24, std::string(1, order.volume_condition));
                    insert.bind(25, std::string(1, order.time_condition));
                    insert.bind(26, std::to_string(order.parent_id));
                    insert.exec();
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to add order[{}], exception: {}", to_string(order), e.what());
                }
            }

            const msg::data::Order OrderStorage::get_order(uint64_t order_id)
            {
                msg::data::Order result = {};
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM orders WHERE order_id = ?");
                    query.bind(1, std::to_string(order_id));
                    if (query.executeStep())
                    {
                        result.order_id = std::stoll(query.getColumn(0).getString());
                        result.rcv_time = query.getColumn(1);
                        result.insert_time = query.getColumn(2);
                        result.update_time = query.getColumn(3);
                        strcpy(result.trading_day, query.getColumn(4));
                        strcpy(result.instrument_id, query.getColumn(5));
                        strcpy(result.exchange_id, query.getColumn(6));
                        strcpy(result.account_id, query.getColumn(7));
                        strcpy(result.client_id, query.getColumn(8));
                        result.instrument_type = query.getColumn(9)[0];
                        result.limit_price = query.getColumn(10);
                        result.frozen_price = query.getColumn(11);
                        result.volume = query.getColumn(12);
                        result.volume_traded = query.getColumn(13);
                        result.volume_left = query.getColumn(14);
                        result.tax = query.getColumn(15);
                        result.commission = query.getColumn(16);
                        result.status = query.getColumn(17)[0];
                        result.error_id = query.getColumn(18);
                        strcpy(result.error_msg, query.getColumn(19));
                        result.side = query.getColumn(20)[0];
                        result.offset = query.getColumn(21)[0];
                        result.price_type = query.getColumn(22)[0];
                        result.volume_condition = query.getColumn(23)[0];
                        result.time_condition = query.getColumn(24)[0];
                        result.parent_id = std::stoll(query.getColumn(25).getString());
                    }
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to query order[{}], exception: {}", order_id, e.what());
                }
                return result;
            }

            TradeStorage::TradeStorage(const std::string &file_path) : DatabaseHolder(file_path)
            {
                try
                {
                    std::string sql = "CREATE TABLE IF NOT EXISTS trade(\n"
                                      "                    id              INTEGER PRIMARY KEY NOT NULL,\n"
                                      "                    order_id        CHAR(50),\n"
                                      "                    trade_time      INTEGER,\n"
                                      "                    instrument_id   CHAR(50),\n"
                                      "                    exchange_id     CHAR(50),\n"
                                      "                    account_id      CHAR(50),\n"
                                      "                    client_id       CHAR(50),\n"
                                      "                    instrument_type CHAR(1),\n"
                                      "                    side            CHAR(1),\n"
                                      "                    offset          CHAR(1),\n"
                                      "                    price           DOUBLE,\n"
                                      "                    volume          INTEGER,\n"
                                      "                    tax             DOUBLE,\n"
                                      "                    commission      DOUBLE);";
                    db_.exec(sql);
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to create table trades, exception: {}", e.what());
                }
            }

            long long TradeStorage::add_trade(const msg::data::Trade &trade)
            {
                try
                {
                    SQLite::Statement insert(db_,
                                             "INSERT INTO trade (order_id, trade_time, instrument_id, exchange_id, account_id, client_id, instrument_type, side, offset, price, volume, tax, commission)"
                                             " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                    insert.bind(1, std::to_string(trade.order_id));
                    insert.bind(2, trade.trade_time);
                    insert.bind(3, trade.instrument_id);
                    insert.bind(4, trade.exchange_id);
                    insert.bind(5, trade.account_id);
                    insert.bind(6, trade.client_id);
                    insert.bind(7, std::string(1, trade.instrument_type));
                    insert.bind(8, std::string(1, trade.side));
                    insert.bind(9, std::string(1, trade.offset));
                    insert.bind(10, trade.price);
                    insert.bind(11, trade.volume);
                    insert.bind(12, trade.tax);
                    insert.bind(13, trade.commission);
                    insert.exec();
                    return db_.getLastInsertRowid();
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to insert trade[{}], exception: {}\n", to_string(trade), e.what());
                    return 0;
                }
            }

            AccountListStorage::AccountListStorage(const std::string &file_path) : DatabaseHolder(file_path)
            {
                try
                {
                    std::string sql = "CREATE TABLE IF NOT EXISTS account_list(client_id char(50), account_id char(50) not null, source char(50) not null, type char(1), init_cash double, unique(source, account_id) on conflict ignore);";
                    db_.exec(sql);
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to create table account_list, exception {}", e.what());
                }
            }

            void AccountListStorage::add_account(const std::string &client_id, const std::string &account_id, const std::string &source_id)
            {
                try
                {
                    SQLite::Statement insert(db_, "INSERT INTO account_list (client_id, account_id, source) VALUES(?, ?, ?)");
                    insert.bind(1, client_id);
                    insert.bind(2, account_id);
                    insert.bind(3, source_id);
                    insert.exec();
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to add account({}, {}), exception: {}\n", source_id, account_id, e.what());
                }
            }

            std::vector<msg::data::SubPortfolioInfo> AccountListStorage::get_accounts()
            {
                std::vector<msg::data::SubPortfolioInfo> accounts;
                try
                {
                    SQLite::Statement query(db_, "SELECT * FROM account_list");
                    while (query.executeStep())
                    {
                        msg::data::SubPortfolioInfo account = {};
                        strcpy(account.account_id, query.getColumn(1));
                        strcpy(account.source_id, query.getColumn(2));
                        accounts.push_back(account);
                    }
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to get accounts, exception: {}", e.what());
                }
                return accounts;
            }

            SourceListStorage::SourceListStorage(const std::string &file_path) : DatabaseHolder(file_path)
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

            void SourceListStorage::add_source(const std::string &source)
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

            SnapshotStorage::SnapshotStorage(const std::string &file_path, const std::string &table, bool daily, bool account) :
                DatabaseHolder(file_path), table_(table)
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
                        "close_pnl             double";
                static std::string daily_uid_columns = account ? "account_id, trading_day" : "trading_day";
                static std::string minutely_uid_columns = account ? "account_id, update_time" : "update_time";

                std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS {}({});", table_, columns);
                std::string uid = fmt::format("CREATE UNIQUE INDEX IF NOT EXISTS {}_uid ON {}({});", table_, table_,
                                              daily ? daily_uid_columns : minutely_uid_columns);
                try
                {
                    db_.exec("BEGIN");

                    db_.exec(sql);
                    db_.exec(uid);

                    db_.exec("COMMIT");
                }
                catch (std::exception &e)
                {
                    db_.exec("ROLLBACK");
                    SPDLOG_ERROR("failed to create table {}, \nsql: {}, \nuid: {}, \nexception: {}", table_.c_str(), sql.c_str(), uid.c_str(),
                                 e.what());
                }
            }

            void SnapshotStorage::insert(const msg::data::AccountInfo &account_info)
            {
                SQLite::Statement insert(db_, "REPLACE INTO " + table_ + "(update_time, trading_day, account_id, "
                                                                         "broker_id, source_id, initial_equity, static_equity, dynamic_equity, "
                                                                         "accumulated_pnl, accumulated_pnl_ratio, intraday_pnl, intraday_pnl_ratio, "
                                                                         "avail, market_value, margin, accumulated_fee, intraday_fee, "
                                                                         "frozen_cash, frozen_margin, frozen_fee, position_pnl, close_pnl) "
                                                                         "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                insert.bind(1, account_info.update_time);
                insert.bind(2, account_info.trading_day);
                insert.bind(3, account_info.account_id);
                insert.bind(4, account_info.broker_id);
                insert.bind(5, account_info.source_id);
                insert.bind(6, account_info.initial_equity);
                insert.bind(7, account_info.static_equity);
                insert.bind(8, account_info.dynamic_equity);
                insert.bind(9, account_info.accumulated_pnl);
                insert.bind(10, account_info.accumulated_pnl_ratio);
                insert.bind(11, account_info.intraday_pnl);
                insert.bind(12, account_info.intraday_pnl_ratio);
                insert.bind(13, account_info.avail);
                insert.bind(14, account_info.market_value);
                insert.bind(15, account_info.margin);
                insert.bind(16, account_info.accumulated_fee);
                insert.bind(17, account_info.intraday_fee);
                insert.bind(18, account_info.frozen_cash);
                insert.bind(19, account_info.frozen_margin);
                insert.bind(20, account_info.frozen_fee);
                insert.bind(21, account_info.position_pnl);
                insert.bind(22, account_info.close_pnl);

                try
                {
                    insert.exec();
                }
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to insert into table {}, exception: {}", table_.c_str(), e.what());
                }
            }

            void SnapshotStorage::insert(const msg::data::PortfolioInfo &portfolio_info)
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
                catch (std::exception &e)
                {
                    SPDLOG_ERROR("failed to insert into table {}, exception: {}", table_.c_str(), e.what());
                }
            }
        }
    }
}

