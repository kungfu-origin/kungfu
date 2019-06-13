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
                    page_provider_->release_page(current_page_->get_id());
                }
            }

            void journal::next()
            {
                assert(current_page_.get() != nullptr);

                frame_.move_to_next();
                if (frame_.address() > current_page_->address_border())
                {
                    load_next_page();
                }
            }

            void journal::seek_to_time(int64_t nanotime)
            {
                int possible_id = page::find_page_id(page_provider_->get_location(), nanotime);
                if (current_page_id_ != possible_id)
                {
                    current_page_ = page_provider_->get_page(possible_id, current_page_id_);
                    current_page_id_ = current_page_->get_id();
                    frame_.set_address(current_page_->first_frame_address());
                }
                SPDLOG_TRACE("seek time {} in current page [{} - {}]",
                        nanotime > 0 ? time::strftime(nanotime) : "",
                        time::strftime(current_page_->begin_time()),
                        time::strftime(current_page_->end_time())
                        );
                while (current_page_->is_full() && current_page_->end_time() <= nanotime)
                {
                    load_next_page();
                }
                while (frame_.has_data() && frame_.gen_time() <= nanotime)
                {
                    next();
                }
            }

            void journal::load_next_page()
            {
                SPDLOG_TRACE("loading page id {}", current_page_id_);
                current_page_ = page_provider_->get_page(current_page_id_ + 1, current_page_id_);
                current_page_id_ = current_page_->get_id();
                frame_.set_address(current_page_->first_frame_address());
            }
        }
    }
}