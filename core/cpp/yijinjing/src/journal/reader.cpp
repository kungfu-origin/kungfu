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
            reader::reader(page_provider_factory_ptr factory) : factory_(factory)
            {}

            reader::~reader()
            {
                journals_.clear();
            }

            void
            reader::subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time)
            {
                auto provider = factory_->make_page_provider(m, c, group, name, false);
                auto key = provider->get_location().keyname();
                if (journals_.find(key) == journals_.end())
                {
                    current_ = std::make_shared<journal>(provider);
                    current_->seek_to_time(from_time);
                    journals_[key] = current_;
                    seek_current_journal();
                }
            }

            void reader::seek_to_time(int64_t nanotime)
            {
                for (std::pair<std::string, journal_ptr> element: journals_)
                {
                    element.second->seek_to_time(nanotime);
                }
                seek_current_journal();
            }

            void reader::next()
            {
                assert(current_.get() != nullptr);
                current_->next();
                seek_current_journal();
            }

            void reader::seek_current_journal()
            {
                if (journals_.size() == 1)
                {
                    return;
                }

                int64_t min_time = time::now_in_nano();
                for (std::pair<std::string, journal_ptr> element: journals_)
                {
                    auto frame = element.second->current_frame();
                    if (frame.has_data() && frame.gen_time() <= min_time)
                    {
                        min_time = frame.gen_time();
                        current_ = element.second;
                    }
                }
            }

        }
    }
}