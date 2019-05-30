#include <utility>

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

#ifndef YIJINJING_JOURNAL_H
#define YIJINJING_JOURNAL_H

#include <mutex>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/frame.h>

namespace kungfu {

    namespace yijinjing {

        namespace journal {

            class session {
            public:
                session(const page_fileinfo &info, int64_t start, int sid, int64_t end, int eid, bool c):
                    fileinfo(&info), start_time(start), start_page_id(sid), end_time(end), end_page_id(eid), closed(c) {}
                const page_fileinfo *fileinfo;
                const int64_t start_time;
                const int start_page_id;
                const int64_t end_time;
                const int end_page_id;
                const bool closed;
            };
            DECLARE_PTR(session)

            /**
             * Journal class, the abstraction of continuous memory access
             */
            class journal {
            public:
                journal(page_provider_ptr provider) : page_provider_(provider) {}

                inline frame &current_frame() { return current_page_->current_frame(); }

                void seek_next_frame();

                /** seek to time in nanoseconds-timestamp */
                void seek_to_time(int64_t nanotime);

                /** load next page, current page will be released if not empty */
                void load_next_page();

                const std::vector<session_ptr> &get_sessions();

            private:
                page_provider_ptr page_provider_;
                page_ptr current_page_;
                std::vector<session_ptr> sessions_;
            };

            class reader {
            public:
                explicit reader() {};

                /** subscribe to a given journal  */
                virtual void subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time) = 0;

                virtual frame& current_frame() = 0;
                /** seek journal to time */
                virtual void seek_to_time(int64_t time) = 0;
                /** seek next frame */
                virtual void seek_next() = 0;

                virtual const std::vector<session_ptr> &get_sessions() = 0;
            };
            DECLARE_PTR(reader)

            class single_reader : public reader {
            public:
                explicit single_reader(page_provider_factory_ptr factory): factory_(factory) {};
                void subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time) override;
                inline frame& current_frame() override { return journal_->current_frame(); }
                void seek_to_time(int64_t time) override ;
                void seek_next() override ;
                const std::vector<session_ptr> &get_sessions() override { return journal_->get_sessions(); };

            private:
                journal_ptr journal_;
                page_provider_ptr page_provider_;
                page_provider_factory_ptr factory_;
            };
            DECLARE_PTR(single_reader)

            class aggregate_reader : public reader {
            public:
                explicit aggregate_reader(page_provider_factory_ptr factory): factory_(factory) {};
                void subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time) override;
                frame& current_frame() override { return current_->current_frame(); }
                void seek_to_time(int64_t time) override ;
                void seek_next() override ;
                const std::vector<session_ptr> &get_sessions() override ;

            private:
                page_provider_factory_ptr factory_;
                single_reader_ptr current_;
                std::unordered_map<std::string, single_reader_ptr> readers_;
                std::vector<session_ptr> sessions_;
                void seek_current_reader();
            };

            class writer {
            public:
                explicit writer(page_provider_factory_ptr factory,
                        data::mode m, data::category c, const std::string &group, const std::string& name,
                        nanomsg::master_messenger_ptr messenger);

                frame& open_frame(int16_t source, int16_t msg_type, int64_t trigger_time);
                void close_frame(int32_t length);

                template <typename T>
                inline void write(int16_t source, int16_t msg_type, int64_t trigger_time, const T * data) {
                    frame &frame = open_frame(source, msg_type, trigger_time);
                    close_frame(frame.copy_data<T>(data));
                }

            private:
                std::mutex writer_mtx_;
                nanomsg::master_messenger_ptr messenger_;
                journal_ptr journal_;
            };
            DECLARE_PTR(writer)
        }
    }
}
#endif //YIJINJING_JOURNAL_H
