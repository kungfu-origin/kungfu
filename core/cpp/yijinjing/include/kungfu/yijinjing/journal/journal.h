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

#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/journal/page.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {

            FORWARD_DECLARE_PTR(page_provider)

            FORWARD_DECLARE_PTR(page_provider_factory)

            /**
             * Journal class, the abstraction of continuous memory access
             */
            class journal
            {
            public:
                journal(const data::location_ptr location, uint32_t dest_id, bool is_writing, bool lazy) :
                    location_(location), dest_id_(dest_id), is_writing_(is_writing), lazy_(lazy), frame_(std::shared_ptr<frame>(new frame()))
                {}

                ~journal();

                frame_ptr current_frame()
                { return frame_; }

                /**
                 * move current frame to the next available one
                 */
                void next();

                /**
                 * makes sure after this call, next time user calls current_frame() gets the right available frame
                 * (gen_time() > nanotime or writable)
                 * @param nanotime
                 */
                void seek_to_time(int64_t nanotime);

            private:
                const data::location_ptr location_;
                const uint32_t dest_id_;
                const bool is_writing_;
                const bool lazy_;
                page_ptr current_page_;
                frame_ptr frame_;

                void load_page(int page_id);

                /** load next page, current page will be released if not empty */
                void load_next_page();

                /** writer needs access to current_page_ to update page header */
                friend class reader;
                friend class writer;
            };

            class reader
            {
            public:
                reader(bool lazy) : lazy_(lazy) {};

                ~reader();

                /**
                 * subscribe to specified data location
                 * @param m mode
                 * @param c category
                 * @param group group
                 * @param name name
                 * @param from_time subscribe events after this time, 0 means from start
                 */
                void subscribe(const data::location_ptr location, uint32_t dest_id, const int64_t from_time);

                inline frame_ptr current_frame()
                { return current_->current_frame(); }

                inline bool data_available()
                { return current_.get() != nullptr && current_frame()->has_data(); }

                /** seek journal to time */
                void seek_to_time(int64_t nanotime);

                /** seek next frame */
                void next();

            private:
                const bool lazy_;
                journal_ptr current_;
                std::vector<journal_ptr> journals_;

                void seek_current_journal();
            };

            class writer
            {
            public:
                explicit writer(const data::location_ptr location, uint32_t dest_id, bool lazy, publisher_ptr messenger);

                frame_ptr open_frame(int64_t trigger_time, int16_t msg_type, int16_t source);

                void close_frame(size_t data_length);

                template<typename T>
                inline void write(int64_t trigger_time, int16_t msg_type, int16_t source, const T *data)
                {
                    auto frame = open_frame(trigger_time, msg_type, source);
                    close_frame(frame->copy_data<T>(data));
                }

                void write_raw(int64_t trigger_time, int16_t msg_type, int16_t source, char *data, int32_t length);

                void open_session();

                void close_session();

            private:
                std::mutex writer_mtx_;
                journal_ptr journal_;
                publisher_ptr publisher_;
            };
        }
    }
}
#endif //YIJINJING_JOURNAL_H
