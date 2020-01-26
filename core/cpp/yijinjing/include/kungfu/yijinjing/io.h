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

//
// Created by Keren Dong on 2019-06-01.
//

#ifndef KUNGFU_IO_H
#define KUNGFU_IO_H

#include <sqlite3.h>

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

#include <utility>

namespace kungfu
{

    namespace yijinjing
    {

        FORWARD_DECLARE_PTR(session)

        class io_device
        {
        public:

            io_device(data::location_ptr home, bool low_latency, bool lazy, bool unique = true);

            virtual ~io_device();

            [[nodiscard]] data::location_ptr get_home() const
            { return home_; }

            [[nodiscard]] data::location_ptr get_live_home() const
            { return live_home_; }

            bool is_low_latency()
            { return low_latency_; }

            journal::reader_ptr open_reader_to_subscribe();

            journal::reader_ptr open_reader(const data::location_ptr &location, uint32_t dest_id);

            journal::writer_ptr open_writer(uint32_t dest_id);

            journal::writer_ptr open_writer_at(const data::location_ptr &location, uint32_t dest_id);

            nanomsg::socket_ptr
            connect_socket(const data::location_ptr &location, const nanomsg::protocol &p, int timeout = 0);

            nanomsg::socket_ptr
            bind_socket(const nanomsg::protocol &p, int timeout = 0);

            [[nodiscard]] nanomsg::url_factory_ptr get_url_factory() const
            { return url_factory_; }

            publisher_ptr get_publisher()
            { return publisher_; }

            observer_ptr get_observer()
            { return observer_; }

            [[nodiscard]] std::vector<std::string> find_sessions(uint32_t source = 0, int64_t from = 0, int64_t to = INT64_MAX) const;

            static void init_sqlite();

            static void shutdown_sqlite();

        protected:
            data::location_ptr home_;
            data::location_ptr db_home_;
            data::location_ptr live_home_;
            const bool low_latency_;
            const bool lazy_;
            const bool unique_;
            nanomsg::url_factory_ptr url_factory_;
            publisher_ptr publisher_;
            observer_ptr observer_;

            sqlite3 *index_db_ = nullptr;
            char *db_error_msg_ = nullptr;
            sqlite3_session *db_session_ = nullptr;
            int db_changeset_nb_ = 0;
            void *db_changeset_ptr_ = nullptr;
            sqlite3_stmt *stmt_find_sessions_ = nullptr;
        };

        DECLARE_PTR(io_device)

        class io_device_with_reply : public io_device
        {
        public:

            io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy);

            [[nodiscard]] nanomsg::socket_ptr get_rep_sock() const
            { return rep_sock_; }

        protected:
            nanomsg::socket_ptr rep_sock_;
        };

        DECLARE_PTR(io_device_with_reply)

        class io_device_master : public io_device_with_reply
        {
        public:
            io_device_master(data::location_ptr home, bool low_latency);

            void open_session(const data::location_ptr &location, int64_t time);

            void update_session(const journal::frame_ptr &frame);

            void close_session(const data::location_ptr &location, int64_t time);

            void rebuild_index_db();

        private:
            std::unordered_map<uint32_t, session_ptr> sessions_;
            sqlite3_stmt *stmt_clean_sessions_;
            sqlite3_stmt *stmt_open_session_;
            sqlite3_stmt *stmt_close_session_;
        };

        DECLARE_PTR(io_device_master)

        class io_device_client : public io_device_with_reply
        {
        public:
            io_device_client(data::location_ptr home, bool low_latency);
        };

        DECLARE_PTR(io_device_client)

        class session
        {
        public:
            explicit session(data::location_ptr location): location_(std::move(location))
            {}

            ~session() = default;

            void update(const journal::frame_ptr &frame);

        private:
            const data::location_ptr location_;
            int64_t begin_time_ = 0;
            int64_t end_time_ = -1;
            int frame_count_ = 0;
            uint64_t data_size_ = 0;
            std::vector<uint32_t> write_to_;
            std::vector<uint32_t> read_from_;

            friend io_device_master;
        };
    }
}
#endif //KUNGFU_IO_H
