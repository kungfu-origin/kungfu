//
// Created by qlu on 2019/5/22.
//

#include "storage.h"

namespace kungfu
{
    namespace portfolio_util
    {
        void create_pos_tables(SQLite::Database& db)
        {
            for(const std::string& table : std::vector<std::string>({"position", "bond_position", "pos_detail"}))
            {
                std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS {}("
                                                      "rcv_time              INTEGER, "
                                                      "update_time           INTEGER, "
                                                      "instrument_id         CHAR(50), "
                                                      "instrument_type       CHAR(1), "
                                                      "exchange_id           CHAR(50), "
                                                      "account_id            CHAR(50), "
                                                      "client_id             CHAR(50), "
                                                      "direction             CHAR(1), "
                                                      "volume                INTEGER, "
                                                      "yesterday_volume      INTEGER, "
                                                      "frozen_total          INTEGER, "
                                                      "frozen_yesterday      INTEGER, "
                                                      "last_price            DOUBLE, "
                                                      "open_price            DOUBLE, "
                                                      "cost_price            DOUBLE, "
                                                      "close_price           DOUBLE, "
                                                      "pre_close_price       DOUBLE, "
                                                      "settlement_price      DOUBLE, "
                                                      "pre_settlement_price  DOUBLE, "
                                                      "margin                DOUBLE, "
                                                      "position_pnl          DOUBLE, "
                                                      "close_pnl             DOUBLE, "
                                                      "realized_pnl          DOUBLE, "
                                                      "unrealized_pnl        DOUBLE, "
                                                      "open_date             CHAR(50), "
                                                      "expire_date           CHAR(50)"
                                                      ")",
                                              table);
                db.exec(sql);
            }
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS position_uid ON position("
                            "instrument_id, exchange_id, instrument_type, account_id, client_id, direction"
                            ")");
            db.exec("CREATE TABLE IF NOT EXISTS pos_frozen(order_id INTEGER PRIMARY KEY, account_id CHAR(50), frozen_vol INTEGER)");
            db.exec("CREATE TABLE IF NOT EXISTS meta(update_time INTEGER, trading_day CHAR(50))");
        }

        void create_acc_tables(SQLite::Database& db)
        {
            db.exec("CREATE TABLE IF NOT EXISTS account("
                            "rcv_time int, "
                            "update_time int, "
                            "trading_day char(50), "
                            "account_id char(50), "
                            "type char(1), "
                            "broker_id char(50), "
                            "source_id char(50), "
                            "initial_equity double, "
                            "static_equity double, "
                            "dynamic_equity double, "
                            "accumulated_pnl double, "
                            "accumulated_pnl_ratio double, "
                            "intraday_pnl double, "
                            "intraday_pnl_ratio double, "
                            "avail double, "
                            "market_value double, "
                            "margin double, "
                            "accumulated_fee double, "
                            "intraday_fee double, "
                            "frozen_cash char(50), "
                            "frozen_margin char(50), "
                            "frozen_fee char(50), "
                            "position_pnl char(50), "
                            "close_pnl char(50)"
                            ")");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS account_uid ON account(account_id, source_id)");
            db.exec("CREATE TABLE IF NOT EXISTS bond_expire(date char(50), account_id char(50), amount double)");
            db.exec("CREATE UNIQUE INDEX IF NOT EXISTS bond_expire_uid ON bond_expire(date, account_id)");
            db.exec("CREATE TABLE IF NOT EXISTS acc_frozen(order_id int PRIMARY KEY, account_id char(50), amount double)");
        }

        void create_pnl_tables(SQLite::Database& db)
        {
            db.exec("CREATE TABLE IF NOT EXISTS portfolio("
                            "trading_day char(50), "
                            "update_time int, "
                            "initial_equity double, "
                            "static_equity double, "
                            "dynamic_equity double, "
                            "accumulated_pnl double, "
                            "accumulated_pnl_ratio double, "
                            "intraday_pnl double, "
                            "intraday_pnl_ratio double "
                            ")");
        }

        void create_meta_table(SQLite::Database& db)
        {
            db.exec("CREATE TABLE IF NOT EXISTS meta(update_time, trading_day char(50))");
        }

        void save_meta_inner(SQLite::Database& db, int64_t last_update, const std::string& trading_day)
        {
            SQLite::Statement delete_meta(db, "DELETE FROM meta");
            delete_meta.exec();

            SQLite::Statement insert_meta(db, "INSERT INTO meta(update_time, trading_day) VALUES(?, ?)");
            insert_meta.bind(1, last_update);
            insert_meta.bind(2, trading_day);
            insert_meta.exec();
        }

        void load_meta_inner(SQLite::Database& db, int64_t& last_update, std::string& trading_day)
        {
            SQLite::Statement query(db, "SELECT * from meta");
            if (query.executeStep())
            {
                last_update = query.getColumn(0);
                trading_day = query.getColumn(1).getString();
            }
        }

        void init_db(const char* db_file)
        {
            SQLite::Database db(db_file, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Transaction tr(db);
            create_meta_table(db);
            create_pnl_tables(db);
            create_acc_tables(db);
            create_pos_tables(db);
            tr.commit();
        }
    }
}
