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
                /** version of frame header */
                int16_t version;
                int16_t header_length;
                /** how many frame in this page (only filled when closed) */
                int32_t frame_count;
                /** pos of last frame */
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
                { return id_; };

                inline const data::location &get_location() const
                { return location_; };

                inline frame &current_frame()
                { return frame_; };

                inline uintptr_t address() const
                { return reinterpret_cast<uintptr_t>(header_); }

                inline uintptr_t address_border() const
                { return address() + JOURNAL_PAGE_SIZE - PAGE_MIN_HEADROOM; };

                inline int64_t begin_time() const
                { return first_frame_header()->gen_time; };

                inline int64_t end_time() const
                { return last_frame_header()->gen_time; };

                inline bool reached_end() const
                {
                    return frame_.address() > address_border();
                }

                /**
                 * seek next frame, user is responsible to check reached_end
                 */
                void seek_next();

                /**
                 * seek to the first writable frame
                 * @return true if found a writable frame, false if the page is full
                 */
                bool seek_to_writable();

                /**
                 * seek to the next frame that passed given time
                 * @param nanotime time in nanoseconds
                 * @return true if found a frame, false if no frame found
                 */
                bool seek_to_time(int64_t nanotime);

                void release();

                static page_ptr load(const data::location &location, int page_id, bool is_writing = false, bool lazy = true);

                static std::string get_page_path(const data::location &location, int id);

                static std::vector<int> list_page_ids(const data::location &location);

                static int find_page_id(const data::location &location, int64_t time);

            private:
                const data::location &location_;
                const int id_;
                const bool lazy_;
                const size_t size_;
                page_header *header_;
                frame frame_;
                int frame_count_;

                page(const data::location &location, int id, size_t size, bool lazy, page_header *header);

                inline frame_header *first_frame_header() const
                { return reinterpret_cast<frame_header *>(address() + header_->header_length); };

                inline frame_header *last_frame_header() const
                { return reinterpret_cast<frame_header *>(address() + header_->last_frame_position); };

                void on_frame_added();
                friend class writer;
            };
        }
    }
}

#endif //YIJINJING_PAGE_H
