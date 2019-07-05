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

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/msg.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace journal
        {

            journal::~journal()
            {
                if (current_page_.get() != nullptr)
                {
                    current_page_->release();
                }
            }

            void journal::next()
            {
                assert(current_page_.get() != nullptr);

                frame_->move_to_next();
                page_frame_nb_++;

                if (frame_->msg_type() == msg::type::PageEnd)
                {
                    load_next_page();
                }
            }

            void journal::seek_to_time(int64_t nanotime)
            {
                int page_id = page::find_page_id(location_, dest_id_, nanotime);
                load_page(page_id);
                SPDLOG_TRACE("{} in page [{}] [{} - {}]",
                             nanotime > 0 ? time::strftime(nanotime) : "beginning", page_id,
                             time::strftime(current_page_->begin_time(), "%F %T"), time::strftime(current_page_->end_time(), "%F %T"));
                while (current_page_->is_full() && current_page_->end_time() <= nanotime)
                {
                    SPDLOG_CRITICAL("{} [{}] is full, {}", location_->uname, current_page_->page_id_, current_page_->header_->last_frame_position);
                    int i = 1;
                    while (frame_->has_data())
                    {
                        SPDLOG_INFO("[{:04d}] {} {} pp{} fp{}, [{}] {} - {} = {}", i, time::strftime(frame_->gen_time()), frame_->frame_length(), current_page_->header_->frame_header_length, frame_->header_->length,
                                    frame_->address() + frame_->frame_length(), frame_->address(), current_page_->address(),
                                    frame_->address() - current_page_->address());
                        frame_->move_to_next();
                        i++;
                    }
                    load_next_page();
                }
                while (frame_->has_data() && frame_->gen_time() <= nanotime)
                {
                    next();
                }
            }

            void journal::load_page(int page_id)
            {
                if (current_page_.get() != nullptr)
                {
                    current_page_->release();
                }
                if (current_page_.get() == nullptr or current_page_->get_page_id() != page_id)
                {
                    current_page_ = page::load(location_, dest_id_, page_id, is_writing_, lazy_);
                    frame_->set_address(current_page_->first_frame_address());
                    page_frame_nb_ = 0;
                }
            }

            void journal::load_next_page()
            {
                load_page(current_page_->get_page_id() + 1);
            }
        }
    }
}