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

#include <sstream>
#include <assert.h>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/io.h>

using namespace kungfu::yijinjing::journal;

void single_reader::subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time)
{
    if (journal_.get() == nullptr)
    {
        page_provider_ = factory_->make_page_provider(m, c, group, name, false);
        SPDLOG_DEBUG("subscribe to {}", page_provider_->get_fileinfo().dir);
        journal_ = std::make_shared<journal>(page_provider_);
        journal_->seek_to_time(from_time);
        SPDLOG_DEBUG("subscribe from time {}", journal_->current_frame().gen_time());
    } else
    {
        throw exception("single_reader can only subscribe one journal");
    }
}

void single_reader::seek_to_time(int64_t time)
{
    journal_->seek_to_time(time);
}

void single_reader::seek_next()
{
    journal_->seek_next_frame();
}

void aggregate_reader::subscribe(data::mode m, data::category c, const std::string &group, const std::string &name, const int64_t from_time)
{
    std::string journal_name = data::get_mode_name(m) + data::get_category_name(c) + group + name;
    if (readers_.find(journal_name) == readers_.end())
    {
        auto reader = std::make_shared<single_reader>(factory_);
        reader->subscribe(m, c, group, name, from_time);
        readers_[journal_name] = reader;
        seek_current_reader();
    }
}

void aggregate_reader::seek_to_time(int64_t time)
{
    for (std::pair<std::string, single_reader_ptr> element: readers_)
    {
        element.second->seek_to_time(time);
    }
    seek_current_reader();
}

void aggregate_reader::seek_next()
{
    current_->seek_next();
    seek_current_reader();
}

const std::vector<session_ptr> &aggregate_reader::get_sessions()
{
    sessions_.clear();
    for (std::pair<std::string, single_reader_ptr> element: readers_)
    {
        auto s = element.second->get_sessions();
        sessions_.insert(sessions_.end(), s.begin(), s.end());
    }
    return sessions_;
}

void aggregate_reader::seek_current_reader()
{
    int64_t min_time = time::now_in_nano();
    for (std::pair<std::string, single_reader_ptr> element: readers_)
    {
        int64_t frame_time = element.second->current_frame().gen_time();
        if (frame_time <= min_time)
        {
            min_time = frame_time;
            current_ = element.second;
        }
    }
}
