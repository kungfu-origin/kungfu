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
#include <signal.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/time.h>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page_service.h>

using namespace kungfu::yijinjing::journal::paged;

using json = nlohmann::json;

page_service::page_service(io_device_ptr io_device) : io_device_(io_device), memory_message_limit_(0)
{
    memory_msg_file_ = os::make_path({KF_DIR_JOURNAL, PAGE_SERVICE_MSG_FILE}, true);
    memory_message_buffer_ = os::load_mmap_buffer(memory_msg_file_, memory_msg_file_size_, true, true);
    memset(reinterpret_cast<void *>(memory_message_buffer_), 0, memory_msg_file_size_);
    SPDLOG_INFO("Loaded page service memory msg buffer: {}", memory_msg_file_);
}

std::string page_service::get_memory_msg_file()
{
    return memory_msg_file_;
}

size_t page_service::get_memory_msg_file_size()
{
    return memory_msg_file_size_;
}

int page_service::get_mm_block(int8_t mode, int8_t category, const std::string &group, const std::string &name, bool is_writing)
{
    int idx = 0;
    for (; idx < MAX_MEMORY_MSG_NUMBER; idx++)
    {
        auto status = static_cast<state>(get_page_request(memory_message_buffer_, idx)->status);
        if (status == state::IDLE)
        {
            break;
        }
    }

    if (idx >= MAX_MEMORY_MSG_NUMBER)
    {
        SPDLOG_ERROR("idx {} exceeds limit {}", idx, MAX_MEMORY_MSG_NUMBER);
        return -1;
    }

    auto factory = io_device_->get_page_provider_factory();
    // master always use writer mode for page manipulation
    auto page_provider = factory->make_page_provider(static_cast<data::mode>(mode), static_cast<data::category>(category), group, name, true);
    if (is_writing)
    {
        auto key = page_provider->get_location().keyname();
        bool has_writer = writers_.find(key) != writers_.end() && writers_[key];
        if (has_writer)
        {
            SPDLOG_ERROR("registering more than one writer for {}", key);
            return -1;
        } else
        {
            writers_[key] = true;
        }
    }

    if (idx >= memory_message_limit_)
    {
        memory_message_limit_ = idx + 1;
    }
    page_providers_[idx] = page_provider;
    page_request *msg = get_page_request(memory_message_buffer_, idx);
    msg->status = state::WAITING;
    SPDLOG_INFO("Register journal for {}/{} with id {}", page_provider->get_location().group, name, idx);
    return idx;
}

void page_service::release_mm_block(int id)
{
    auto page_provider = page_providers_[id];
    page_provider->release_all();
    if (page_provider->is_writing())
    {
        auto key = page_provider->get_location().keyname();
        writers_[key] = false;
    }
    page_providers_.erase(id);
}

void page_service::process_memory_message()
{
    for (int32_t msg_id = 0; msg_id < memory_message_limit_; msg_id++)
    {
        page_request *msg = get_page_request(memory_message_buffer_, msg_id);
        switch (msg->status)
        {
            case state::REQUESTING:
            {
                auto page_provider = page_providers_[msg_id];
                SPDLOG_INFO("Request page for {} {} [{}/{}]", page_provider->get_location().group, page_provider->get_location().name, msg->new_page_id, msg->old_page_id);
                page_provider->get_page(msg->new_page_id, -1);
                msg->status = state::WAITING;
                if (msg->old_page_id >= 0)
                {
                    page_provider->release_page(msg->old_page_id);
                }
                break;
            }
            case state::RELEASING:
            {
                auto page_provider = page_providers_[msg_id];
                page_provider->release_page(msg->old_page_id);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}
