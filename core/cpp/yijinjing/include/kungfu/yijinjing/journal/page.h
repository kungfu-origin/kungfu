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

#ifndef YIJINJING_PAGE_H
#define YIJINJING_PAGE_H

#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/journal/frame.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace journal
        {

            struct page_header
            {
                int32_t version;
                int32_t page_header_length;
                int32_t frame_header_length;
                int32_t last_frame_position;
#ifndef _WIN32
            } __attribute__((packed));
#else
            };
#pragma pack(pop)
#endif

            class page
            {
            public:

                inline const int get_id() const
                { return id_; }

                inline const data::location &get_location() const
                { return location_; }

                inline int64_t begin_time() const
                { return reinterpret_cast<frame_header *>(first_frame_address())->gen_time; }

                inline int64_t end_time() const
                { return reinterpret_cast<frame_header *>(last_frame_address())->gen_time; }

                inline uintptr_t address() const
                { return reinterpret_cast<uintptr_t>(header_); }

                inline uintptr_t address_border() const
                { return address() + JOURNAL_PAGE_SIZE - PAGE_MIN_HEADROOM; }

                inline uintptr_t first_frame_address() const
                { return address() + header_->page_header_length; };

                inline uintptr_t last_frame_address() const
                { return address() + header_->last_frame_position; };

                inline bool is_full() const
                { return last_frame_address() + reinterpret_cast<frame_header *>(last_frame_address())->length > address_border(); }

                void release();

                static page_ptr load(const data::location &location, int page_id, bool is_writing = false, bool lazy = true);

                static std::string get_page_path(const data::location &location, int id);

                static int find_page_id(const data::location &location, int64_t time);

            private:

                const data::location &location_;
                const int id_;
                const bool lazy_;
                const size_t size_;
                const page_header *header_;

                page(const data::location &location, int id, size_t size, bool lazy, uintptr_t address);

                /**
                 * update page header when new frame added
                 */
                void set_last_frame_position(int32_t position);

                friend class journal;
                friend class writer;
            };
        }
    }
}

#endif //YIJINJING_PAGE_H
