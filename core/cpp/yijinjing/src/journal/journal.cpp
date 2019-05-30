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

using namespace kungfu::yijinjing::journal;

void journal::seek_next_frame()
{
    if (page_provider_->is_writing())
    {
        while (!current_page_->seek_to_writable())
        {
            load_next_page();
        }
    } else
    {
        current_page_->seek_next();
        if (current_page_->reached_end())
        {
            load_next_page();
        }
    }
}

void journal::seek_to_time(int64_t nanotime)
{
    int page_id = util::find_page_id(page_provider_->get_fileinfo(), nanotime);
    current_page_ = page_provider_->get_page(page_id, -1);
    current_page_->seek_to_time(nanotime);
}

void journal::load_next_page()
{
    current_page_ = page_provider_->get_page(current_page_->get_id() + 1, current_page_->get_id());
}

const std::vector<session_ptr> &journal::get_sessions()
{
    sessions_.clear();
    const page_fileinfo & fileinfo = page_provider_->get_fileinfo();
    auto page_ids = util::list_page_ids(fileinfo);
    for (auto page_id : page_ids)
    {
        auto page = page_provider_->get_page(page_id, -1);
        auto frame = page->current_frame();
        int64_t session_start_time = -1;
        int64_t last_frame_time = -1;
        int session_start_page_id = -1;
        while (frame.has_data())
        {
            auto ts_frame = time::strftime(frame.gen_time());
            auto ts_start = time::strftime(session_start_time);
            switch (frame.msg_type())
            {
                case MsgType::SessionStart:
                {
                    SPDLOG_TRACE("found session start {}, {}, page {}, frame {}", ts_frame, ts_start, page_id, frame.address());
                    if (session_start_time > 0)
                    {
                        sessions_.push_back(std::make_shared<session>(fileinfo, session_start_time, session_start_page_id, frame.gen_time() -1, page_id, false));
                        SPDLOG_DEBUG("found session [{},{})", ts_start, ts_frame);
                        session_start_time = -1;
                    }
                    else
                    {
                        session_start_time = frame.gen_time();
                        session_start_page_id = page_id;
                    }
                    break;
                }
                case MsgType::SessionEnd:
                {
                    SPDLOG_TRACE("found session end {}, page {}, frame {}", ts_frame, page_id, frame.address());
                    if (session_start_time > 0)
                    {
                        sessions_.push_back(std::make_shared<session>(fileinfo, session_start_time, session_start_page_id, frame.gen_time(), page_id, true));
                        SPDLOG_DEBUG("found session [{},{}]", ts_start, ts_frame);
                        session_start_time = -1;
                    }
                    else
                    {
                        SPDLOG_WARN("Unexpected session end {}, page {}", ts_frame, page_id);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            last_frame_time = frame.gen_time();
            page->seek_next();
            frame = page->current_frame();
        }
        if (session_start_time > 0)
        {
            sessions_.push_back(std::make_shared<session>(fileinfo, session_start_time, session_start_page_id, last_frame_time, page_ids.back(), true));
            session_start_time = -1;
        }
    }
    return sessions_;
}
