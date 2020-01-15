/*****************************************************************************
 * Copyright [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <utility>
#include <typeinfo>
#include <spdlog/spdlog.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu
{
    namespace yijinjing
    {
        class ipc_url_factory : public url_factory
        {
        public:

            const std::string make_path_bind(data::location_ptr location, protocol p) const override
            {
                return location->locator->layout_file(location, layout::NANOMSG, get_protocol_name(p));
            }

            const std::string make_path_connect(data::location_ptr location, protocol p) const override
            {
                return location->locator->layout_file(location, layout::NANOMSG, get_protocol_name(get_opposite_protol(p)));
            }
        };

        class nanomsg_publisher : public publisher
        {
        public:
            nanomsg_publisher(bool low_latency, protocol p) : socket_(p), low_latency_(low_latency)
            {}

            ~nanomsg_publisher() override
            {
                socket_.close();
            }

            void init(const io_device &io)
            {
                auto location = std::make_shared<data::location>(data::mode::LIVE, data::category::SYSTEM, "master", "master",
                                                                 io.get_home()->locator);
                init_socket(socket_, location, io.get_url_factory());
            }

            int notify() override
            {
                return low_latency_ ? 0 : publish("{}");
            }

            int publish(const std::string &json_message) override
            {
                return socket_.send(json_message);
            }

        protected:
            virtual void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) = 0;

        private:
            const bool low_latency_;
            socket socket_;
        };

        class nanomsg_publisher_master : public nanomsg_publisher
        {
        public:
            nanomsg_publisher_master(bool low_latency) : nanomsg_publisher(low_latency, protocol::PUBLISH)
            {}

        protected:
            void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) override
            {
                s.bind(url_factory->make_path_bind(location, s.get_protocol()));
            }
        };

        class nanomsg_publisher_client : public nanomsg_publisher
        {
        public:
            nanomsg_publisher_client(bool low_latency) : nanomsg_publisher(low_latency, protocol::PUSH)
            {}

        protected:
            void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) override
            {
                s.connect(url_factory->make_path_connect(location, s.get_protocol()));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        };

        class nanomsg_observer : public observer
        {
        public:
            nanomsg_observer(bool low_latency, protocol p) : low_latency_(low_latency), socket_(p), recv_flags_(low_latency ? NN_DONTWAIT : 0)
            {}

            void init(const io_device &io)
            {
                auto location = std::make_shared<data::location>(data::mode::LIVE, data::category::SYSTEM, "master", "master",
                                                                 io.get_home()->locator);
                init_socket(socket_, location, io.get_url_factory());
                if (not low_latency_)
                {
                    socket_.setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, DEFAULT_NOTICE_TIMEOUT);
                }
            }

            virtual ~nanomsg_observer()
            {
                socket_.close();
            }

            bool wait() override
            {
                return socket_.recv(recv_flags_) > 0;
            }

            const std::string &get_notice() override
            {
                return socket_.last_message();
            }

        protected:

            virtual void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) = 0;

        private:
            const bool low_latency_;
            socket socket_;
            int recv_flags_;
        };

        class nanomsg_observer_master : public nanomsg_observer
        {
        public:
            nanomsg_observer_master(bool low_latency) : nanomsg_observer(low_latency, protocol::PULL)
            {}

        protected:
            void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) override
            {
                s.bind(url_factory->make_path_bind(location, s.get_protocol()));
            }
        };

        class nanomsg_observer_client : public nanomsg_observer
        {
        public:
            nanomsg_observer_client(bool low_latency) : nanomsg_observer(low_latency, protocol::SUBSCRIBE)
            {}

        protected:
            void init_socket(socket &s, location_ptr location, url_factory_ptr url_factory) override
            {
                s.connect(url_factory->make_path_connect(location, s.get_protocol()));
                s.setsockopt_str(NN_SUB, NN_SUB_SUBSCRIBE, "");
            }
        };

        void sqlite3_log(void *callback, int result_code, const char *msg)
        {
            SPDLOG_WARN("[sqlite3] [{}] {}", result_code, msg);
        }

        int callback(void *NotUsed, int argc, char **argv, char **azColName)
        {
            int i;
            for (i = 0; i < argc; i++)
            {
                SPDLOG_INFO("[sqlite3] callback: {} = {}", azColName[i], argv[i] ? argv[i] : "NULL");
            }
            return 0;
        }

        int wal_hook(void *NotUsed, sqlite3 *db, const char *name, int page_nb)
        {
            SPDLOG_INFO("wal committed to {} with page number {}", name, page_nb);
            return 0;
        }

        int xConflict(void *pCtx, int eConflict, sqlite3_changeset_iter *pIter)
        {
            return 0;
        }

        void destruct_str(void *str)
        {
            SPDLOG_INFO("destruct");
        }

        void handle_sql_error(int rc, const std::string &error_tip)
        {
            if (SQLITE_OK != rc)
            {
                SPDLOG_ERROR("sqlite3 rc {}", rc);
                throw yijinjing_error(error_tip);
            }
        }

        void exec_sql(sqlite3 *db, char **db_error_msg, const std::string &sql, const std::string &error_tip)
        {
            int rc = sqlite3_exec(db, sql.c_str(), callback, NULL, db_error_msg);
            handle_sql_error(rc, error_tip);
        }

        void io_device::init_sqlite()
        {
            int rc = sqlite3_config(SQLITE_CONFIG_LOG, sqlite3_log, NULL);
            handle_sql_error(rc, "failed to config sqlite3 log");

            rc = sqlite3_config(SQLITE_CONFIG_MMAP_SIZE, 1048577);
            handle_sql_error(rc, "failed to config sqlite3");

            sqlite3_initialize();
            SPDLOG_TRACE("sqlite initialized");
        }

        void io_device::shutdown_sqlite()
        {
            sqlite3_shutdown();
            SPDLOG_TRACE("sqlite shutdown");
        }

        io_device::io_device(data::location_ptr home, const bool low_latency, const bool lazy, bool unique) : home_(std::move(home)),
                                                                                                              low_latency_(low_latency),
                                                                                                              lazy_(lazy), unique_(unique)
        {
            if (spdlog::default_logger()->name().empty())
            {
                yijinjing::log::setup_log(home_, home_->name);
            }

            if (unique_)
            {
                init_sqlite();
            }

            db_home_ = location::make(mode::LIVE, category::SYSTEM, "journal", "index", home_->locator);
            live_home_ = location::make(mode::LIVE, home_->category, home_->group, home_->name, home_->locator);
            url_factory_ = std::make_shared<ipc_url_factory>();

            auto index_db_file = home_->locator->layout_file(db_home_, layout::SQLITE, "index");

            int rc = sqlite3_open_v2(index_db_file.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
            handle_sql_error(rc, "failed to open index db");

            exec_sql(
                    db_,
                    &db_error_msg_,
                    "PRAGMA journal_mode=WAL;",
                    "failed to set index db to WAL"
            );
            exec_sql(
                    db_,
                    &db_error_msg_,
                    "create table if not exists session (id INTEGER PRIMARY KEY, data JSON);",
                    "failed to create tables"
            );

            rc = sqlite3session_create(db_, "main", &db_session_);
            handle_sql_error(rc, "failed to create index db session");

            rc = sqlite3session_attach(db_session_, nullptr);
            handle_sql_error(rc, "failed to attach index db session");

            sqlite3_prepare_v2(
                    db_,
                    R"(
select data from session
where json_extract(session.data, '$.uid') between ?1 and ?2
and json_extract(session.data, '$.begin_time') >= ?3
and json_extract(session.data, '$.end_time') <= ?4;
)",
                    -1,
                    &stmt_find_sessions_,
                    nullptr
            );
        }

        io_device::~io_device()
        {
            int rc = sqlite3session_changeset(db_session_, &db_changeset_nb_, &db_changeset_ptr_);
            handle_sql_error(rc, "failed to collect index db session changeset");
            SPDLOG_TRACE("index db changeset nb={}", db_changeset_nb_);
//            sqlite3changeset_apply(
//                    db_,
//                    db_changeset_nb_, db_changeset_ptr_,
//                    0, xConflict,
//                    (void*)1
//            );
            sqlite3_finalize(stmt_find_sessions_);
            sqlite3session_delete(db_session_);
            sqlite3_close_v2(db_);
            SPDLOG_TRACE("index db closed");

            if (unique_)
            {
                shutdown_sqlite();
            }
        }

        reader_ptr io_device::open_reader_to_subscribe()
        {
            return std::make_shared<reader>(lazy_);
        }

        reader_ptr io_device::open_reader(const data::location_ptr &location, uint32_t dest_id)
        {
            auto r = std::make_shared<reader>(lazy_);
            r->join(location, dest_id, 0);
            return r;
        }

        writer_ptr io_device::open_writer(uint32_t dest_id)
        {
            return std::make_shared<writer>(home_, dest_id, lazy_, publisher_);
        }

        writer_ptr io_device::open_writer_at(const data::location_ptr &location, uint32_t dest_id)
        {
            return std::make_shared<writer>(location, dest_id, lazy_, publisher_);
        }

        socket_ptr io_device::connect_socket(const data::location_ptr &location, const protocol &p, int timeout)
        {
            socket_ptr s = std::make_shared<socket>(p);
            s->connect(url_factory_->make_path_connect(location, p));
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("connected socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), location->name, s->get_url(), timeout);
            return s;
        }

        socket_ptr io_device::bind_socket(const protocol &p, int timeout)
        {
            socket_ptr s = std::make_shared<socket>(p);
            s->bind(url_factory_->make_path_bind(home_, p));
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("bind to socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), home_->name, s->get_url(), timeout);
            return s;
        }

        std::vector<std::string> io_device::find_sessions(uint32_t source, int64_t from, int64_t to) const
        {
            std::vector<std::string> result;
            sqlite3_reset(stmt_find_sessions_);
            if (source > 0)
            {
                sqlite3_bind_int64(stmt_find_sessions_, 1, source);
                sqlite3_bind_int64(stmt_find_sessions_, 2, source);
            } else
            {
                sqlite3_bind_int64(stmt_find_sessions_, 1, 0);
                sqlite3_bind_int64(stmt_find_sessions_, 2, INT64_MAX);
            }
            sqlite3_bind_int64(stmt_find_sessions_, 3, from);
            sqlite3_bind_int64(stmt_find_sessions_, 4, to);
            SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_find_sessions_));
            int rc = sqlite3_step(stmt_find_sessions_);
            while (SQLITE_ROW == rc)
            {
                result.emplace_back((const char *) sqlite3_column_text(stmt_find_sessions_, 0));
                rc = sqlite3_step(stmt_find_sessions_);
            }
            if (SQLITE_ERROR == rc)
            {
                SPDLOG_ERROR("error occurred when query sessions: {}", sqlite3_errmsg(db_));
            }
            return result;
        }

        io_device_with_reply::io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy) :
                io_device(std::move(home), low_latency, lazy)
        {
            rep_sock_ = std::make_shared<socket>(protocol::REPLY);
            rep_sock_->bind(url_factory_->make_path_bind(home_, protocol::REPLY));
        }

        io_device_master::io_device_master(data::location_ptr home, bool low_latency) : io_device_with_reply(std::move(home), low_latency, false)
        {
            auto publisher = std::make_shared<nanomsg_publisher_master>(low_latency);
            publisher->init(*this);
            publisher_ = publisher;
            auto observer = std::make_shared<nanomsg_observer_master>(low_latency);
            observer->init(*this);
            observer_ = observer;

            sqlite3_prepare_v2(
                    db_,
                    R"(delete from session;)",
                    -1,
                    &stmt_clean_sessions_,
                    NULL
            );
            sqlite3_prepare_v2(
                    db_,
                    R"(insert into session (data) values (json_set('{}', '$.uid', ?1, '$.location', ?2, '$.begin_time', ?3, '$.end_time', 0));)",
                    -1,
                    &stmt_open_session_,
                    NULL
            );
            sqlite3_prepare_v2(
                    db_,
                    R"(
update session set data = json_set(session.data, '$.end_time', ?3, '$.frame_count', ?4, '$.data_size', ?5)
where json_extract(session.data, '$.uid') = ?1 and json_extract(session.data, '$.begin_time') = ?2;
)",
                    -1,
                    &stmt_close_session_,
                    NULL
            );
        }

        void io_device_master::open_session(const location_ptr &location, int64_t time)
        {
            auto uid = location->uid;
            sessions_[uid] = std::make_shared<session>(location);
            sessions_[uid]->begin_time_ = time;

            sqlite3_reset(stmt_open_session_);
            sqlite3_bind_int64(stmt_open_session_, 1, uid);
            sqlite3_bind_text(stmt_open_session_, 2, location->uname.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt_open_session_, 3, time);
            SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_open_session_));
            sqlite3_step(stmt_open_session_);
            SPDLOG_INFO("{} opened session [{}, )", location->uname, time::strftime(time));
        }

        void io_device_master::update_session(const frame_ptr &frame)
        {
            if (sessions_.find(frame->source()) != sessions_.end())
            {
                auto &&session = sessions_[frame->source()];
                session->update(frame);
            } else
            {
                SPDLOG_ERROR("no session found for {:08x}", frame->source());
            }
        }

        void io_device_master::close_session(const data::location_ptr &location, int64_t time)
        {
            auto uid = location->uid;
            auto session = sessions_[uid];
            sqlite3_reset(stmt_close_session_);
            sqlite3_bind_int64(stmt_close_session_, 1, uid);
            sqlite3_bind_int64(stmt_close_session_, 2, session->begin_time_);
            sqlite3_bind_int64(stmt_close_session_, 3, time);
            sqlite3_bind_int(stmt_close_session_, 4, session->frame_count_);
            sqlite3_bind_int(stmt_close_session_, 5, session->data_size_);
            SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_close_session_));
            sqlite3_step(stmt_close_session_);
            SPDLOG_INFO("{} closed session [{}, {}]", session->location_->uname, time::strftime(session->begin_time_), time::strftime(time));
        }

        void io_device_master::rebuild_index_db()
        {
            sqlite3_reset(stmt_clean_sessions_);
            sqlite3_step(stmt_clean_sessions_);

            std::unordered_map<uint32_t, location_ptr> locations;
            std::unordered_set<uint32_t> master_cmd_uids;
            auto reader = open_reader_to_subscribe();
            for (const auto &location: home_->locator->list_locations())
            {
                SPDLOG_TRACE("investigating location {}", location->uname);
                locations[location->uid] = location;
                if (location->category == data::category::SYSTEM && location->group == "master" && location->name != "master")
                {
                    master_cmd_uids.insert(location->uid);
                }
                for (const auto dest_uid : home_->locator->list_location_dest(location))
                {
                    reader->join(location, dest_uid, 0);
                }
            }
            while (reader->data_available())
            {
                auto &&frame = reader->current_frame();
                auto &&location = locations[frame->dest() == 0 ? frame->source() : frame->dest()];
                switch (frame->msg_type())
                {
                    case msg::type::SessionStart:
                    {
                        open_session(location, frame->gen_time());
                        break;
                    }
                    case msg::type::SessionEnd:
                    {
                        close_session(location, frame->gen_time());
                        break;
                    }
                    default:
                    {
                        if (master_cmd_uids.find(frame->source()) == master_cmd_uids.end())
                        {
                            update_session(frame);
                        }
                        break;
                    }
                }
                reader->next();
            }
        }

        io_device_client::io_device_client(data::location_ptr home, bool low_latency) : io_device_with_reply(std::move(home), low_latency, true)
        {
            auto publisher = std::make_shared<nanomsg_publisher_client>(low_latency);
            publisher->init(*this);
            publisher_ = publisher;
            auto observer = std::make_shared<nanomsg_observer_client>(low_latency);
            observer->init(*this);
            observer_ = observer;
        }

        void session::update(const frame_ptr &frame)
        {
            end_time_ = frame->gen_time();
            frame_count_++;
            data_size_ += frame->frame_length();
        }
    }
}


