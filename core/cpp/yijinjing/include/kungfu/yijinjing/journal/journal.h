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
                journal(page_provider_ptr provider) : page_provider_(provider), current_page_id_(-1)
                {}

                ~journal();

                frame &current_frame()
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
                page_provider_ptr page_provider_;
                int current_page_id_;
                page_ptr current_page_;
                frame frame_;

                /** load next page, current page will be released if not empty */
                void load_next_page();

                /** writer needs access to current_page_ to update page header */
                friend class writer;
            };

            class reader
            {
            public:
                reader(page_provider_factory_ptr factory);

                ~reader();

                /**
                 * subscribe to specified data location
                 * @param m mode
                 * @param c category
                 * @param group group
                 * @param name name
                 * @param from_time subscribe events after this time, 0 means from start
                 */
                void subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time);

                inline frame &current_frame()
                { return current_->current_frame(); }

                inline bool data_available()
                { return current_frame().has_data(); }

                /** seek journal to time */
                void seek_to_time(int64_t nanotime);

                /** seek next frame */
                void next();

            private:
                page_provider_factory_ptr factory_;
                journal_ptr current_;
                std::unordered_map<std::string, journal_ptr> journals_;

                void seek_current_journal();
            };

            class writer
            {
            public:
                explicit writer(page_provider_factory_ptr factory,
                                data::mode m, data::category c, const std::string &group, const std::string &name, publisher_ptr messenger);

                frame &open_frame(int64_t trigger_time, int16_t msg_type, int16_t source);

                void close_frame(size_t data_length);

                template<typename T>
                inline void write(int64_t trigger_time, int16_t msg_type, int16_t source, const T *data)
                {
                    frame &frame = open_frame(trigger_time, msg_type, source);
                    close_frame(frame.copy_data<T>(data));
                }

                void write_raw(int64_t trigger_time, int16_t msg_type, int16_t source, char *data, int32_t length);

                void open_session();

                void close_session();

            private:
                std::mutex writer_mtx_;
                journal_ptr journal_;
                publisher_ptr publisher_;
            };

            /** abstract interface class */
            class page_provider
            {
            protected:
                const data::location location_;
                const bool is_writing_;
            public:
                page_provider(data::location &location, const bool is_writing) : location_(std::move(location)), is_writing_(is_writing)
                {};

                /** destructor */
                virtual ~page_provider()
                {};

                const data::location &get_location() const
                { return location_; };

                const bool &is_writing() const
                { return is_writing_; };

                virtual page_ptr get_page(int new_page_id, int old_page_id) = 0;

                virtual void release_page(int page_id) = 0;

                virtual void release_all() = 0;
            };

            class page_provider_factory
            {
            public:
                virtual page_provider_ptr
                make_page_provider(data::mode m, data::category c, const std::string &group, const std::string &name, const bool is_writing) = 0;
            };
        }
    }
}
#endif //YIJINJING_JOURNAL_H
