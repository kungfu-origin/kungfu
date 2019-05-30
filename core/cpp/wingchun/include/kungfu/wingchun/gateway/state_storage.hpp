//
// Created by qlu on 2019/4/1.
//

#include <SQLiteCpp/SQLiteCpp.h>
#include <spdlog/spdlog.h>

#include <kungfu/wingchun/gateway/state.h>

namespace kungfu
{
    class GatewayStateStorage
    {
    public:
        GatewayStateStorage(const std::string& file_name): db_(file_name.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
        {
            create_table_if_not_exist();
        }

        void create_table_if_not_exist()
        {
            try
            {
                std::string sql = "CREATE TABLE IF NOT EXISTS gateway_state("
                        "    name CHAR(50) PRIMARY KEY,\n"
                        "    update_time int,\n"
                        "    state int CHECK(state IN (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 100)),\n"
                        "    message CHAR(200)\n"
                        ");";

                db_.exec(sql);
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR(e.what());
            }
        }

        void set_state(const std::string& name, int64_t update_time, GatewayState state, const std::string& state_message = "")
        {
            try
            {
                SQLite::Statement insert(db_, "INSERT OR REPLACE INTO gateway_state(name, update_time, state, message) VALUES(?, ?, ?, ?)");
                insert.bind(1, name);
                insert.bind(2, update_time);
                insert.bind(3, static_cast<int>(state));
                insert.bind(4, state_message);
                insert.exec();
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR(e.what());
            }
        }

        const GatewayStateRecord get_state(const std::string& name)
        {
            GatewayStateRecord record = {};
            try
            {
                SQLite::Statement query(db_, "SELECT * FROM gateway_state WHERE name == ?");
                query.bind(1, name);
                if (query.executeStep())
                {
                    strcpy(record.name, query.getColumn(0));
                    record.update_time = (long long)query.getColumn(1);
                    int state = query.getColumn(2);
                    record.state = static_cast<GatewayState>(state);
                    strcpy(record.message, query.getColumn(3));
                }
            }
            catch (std::exception &e)
            {
                SPDLOG_ERROR(e.what());
            }
            return record;
        }

    private:
        SQLite::Database db_;
    };
}