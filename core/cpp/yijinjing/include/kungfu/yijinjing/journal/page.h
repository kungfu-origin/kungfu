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
                uint32_t version;
                uint32_t page_header_length;
                uint32_t page_size;
                uint32_t frame_header_length;
                uint64_t last_frame_position;
#ifndef _WIN32
            } __attribute__((packed));
#else
            };
#pragma pack(pop)
#endif

            class page
            {
            public:

                const uint32_t get_page_size() const
                { return header_->page_size; }

                const data::location_ptr get_location() const
                { return location_; }

                const uint32_t get_dest_id() const
                { return dest_id_; }

                const int get_page_id() const
                { return page_id_; }

                int64_t begin_time() const
                { return reinterpret_cast<frame_header *>(first_frame_address())->gen_time; }

                int64_t end_time() const
                { return reinterpret_cast<frame_header *>(last_frame_address())->gen_time; }

                uintptr_t address() const
                { return reinterpret_cast<uintptr_t>(header_); }

                uintptr_t address_border() const
                { return address() + header_->page_size - sizeof(frame_header); }

                uintptr_t first_frame_address() const
                { return address() + header_->page_header_length; }

                uintptr_t last_frame_address() const
                { return address() + header_->last_frame_position; }

                bool is_full() const
                { return last_frame_address() + reinterpret_cast<frame_header *>(last_frame_address())->length > address_border(); }

                void release();

                static page_ptr load(const data::location_ptr& location, uint32_t dest_id, int page_id, bool is_writing, bool lazy);

                static std::string get_page_path(const data::location_ptr& location, uint32_t dest_id, int id);

                static int find_page_id(const data::location_ptr& location, uint32_t dest_id, int64_t time);

            private:

                const data::location_ptr location_;
                const uint32_t dest_id_;
                const int page_id_;
                const bool lazy_;
                const size_t size_;
                const page_header *header_;

                page(const data::location_ptr& location, uint32_t dest_id, int page_id, size_t size, bool lazy, uintptr_t address);

                /**
                 * update page header when new frame added
                 */
                void set_last_frame_position(uint64_t position);

                friend class journal;
                friend class writer;
            };

            inline static uint32_t find_page_size(const data::location_ptr& location, uint32_t dest_id)
            {
                if (location->category == data::category::MD && dest_id == 0)
                {
                    return 128 * MB;
                }
                if ((location->category == data::category::TD || location->category == data::category::STRATEGY) && dest_id != 0)
                {
                    return 4 * MB;
                }
                return MB;
            }
        }
    }
}

#endif //YIJINJING_PAGE_H
