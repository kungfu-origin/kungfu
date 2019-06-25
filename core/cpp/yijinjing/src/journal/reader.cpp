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

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {
            reader::~reader()
            {
                journals_.clear();
            }

            void
            reader::join(const data::location_ptr &location, uint32_t dest_id, const int64_t from_time)
            {
                for (const auto &journal : journals_)
                {
                    if (journal->location_->uid == location->uid && journal->dest_id_ == dest_id)
                    {
                        throw journal_error(fmt::format("reader can not join journal {}/{} more than once", location->uname, dest_id));
                    }
                }
                journals_.push_back(std::make_shared<journal>(location, dest_id, false, lazy_));
                journals_.back()->seek_to_time(from_time);
                if (current_.get() == nullptr)
                {
                    current_ = journals_.back();
                }
            }

            void reader::disjoin(const uint32_t location_uid)
            {
                journals_.erase(std::remove_if(journals_.begin(), journals_.end(),
                                               [&](journal_ptr j)
                                               { return j->location_->uid == location_uid || j->dest_id_ == location_uid; }), journals_.end());
                sort();
            }

            bool reader::data_available()
            {
                sort();
                return current_.get() != nullptr && current_frame()->has_data();
            }

            void reader::seek_to_time(int64_t nanotime)
            {
                for (const auto &journal : journals_)
                {
                    journal->seek_to_time(nanotime);
                }
                sort();
            }

            void reader::next()
            {
                assert(current_.get() != nullptr);
                current_->next();
                sort();
            }

            void reader::sort()
            {
                if (journals_.size() == 1)
                {
                    return;
                }

                int64_t min_time = time::now_in_nano();
                for (const auto &journal : journals_)
                {
                    auto frame = journal->current_frame();
                    if (frame->has_data() && frame->gen_time() <= min_time)
                    {
                        min_time = frame->gen_time();
                        current_ = journal;
                    }
                }
            }

        }
    }
}