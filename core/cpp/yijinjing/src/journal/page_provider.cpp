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

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/journal/page_service.h>

using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::journal::paged;
using namespace kungfu::yijinjing::nanomsg;

using json = nlohmann::json;

page_ptr local_page_provider::get_page(int new_page_id, int old_page_id)
{
    std::string dir = get_fileinfo().locator->get_journal_dir(get_fileinfo().mode, get_fileinfo().category, get_fileinfo().group);
    auto new_page = page::load(get_fileinfo(), new_page_id, is_writing_, lazy_);
    pages_[new_page_id] = new_page;
    if (old_page_id >= 0)
    {
        release_page(old_page_id);
    }
    return new_page;
}

const std::unordered_map<int, page_ptr> &local_page_provider::get_all_pages()
{
    return pages_;
}

void local_page_provider::release_page(int page_id)
{
    pages_[page_id]->release();
    pages_.erase(page_id);
}

void local_page_provider::release_all()
{
    for (auto element : pages_)
    {
        element.second->release();
    }
    pages_.erase(pages_.begin(), pages_.end());
}

client_page_provider::client_page_provider(locator_ptr l, data::mode m, data::category c, const std::string &group, const std::string &name,
                                           const bool is_writing,
                                           const std::string &client_name, nanomsg::master_messenger_ptr messenger) :
        local_page_provider(l, m, c, group, name, is_writing, true), client_name_(client_name), messenger_(messenger)
{
    SPDLOG_TRACE("client_page_provider {} request register", client_name);

#ifdef _WINDOWS
    int pid = _getpid();
#else
    int pid = getpid();
#endif
    nlohmann::json request;
    request["request"] = "journal/register";
    request["pid"] = pid;
    request["mode"] = m;
    request["category"] = c;
    request["group"] = group;
    request["name"] = name;
    request["is_writing"] = is_writing;
    std::string request_str = request.dump();
    std::string response_str = messenger_->get_service()->request(request_str);
    SPDLOG_TRACE("got response: {}", response_str);
    json response = json::parse(response_str);

    if (response["success"])
    {
        int size = response["file_size"];
        std::string memory_msg_file = response["memory_msg_file"];
        memory_msg_id_ = response["memory_msg_id"];
        memory_msg_buffer_ = util::load_mmap_buffer(memory_msg_file, size, true, false);
    } else
    {
        std::string error_msg = response["error_msg"];
        SPDLOG_ERROR("failed to register client {}, error: {}", client_name_, error_msg);
        throw std::runtime_error("cannot register client: " + client_name_);
    }

    page_request *server_msg = get_page_request(memory_msg_buffer_, memory_msg_id_);
    if (server_msg->status != state::WAITING)
    {
        throw std::runtime_error("server buffer is not allocated: " + client_name_);
    }
}

page_ptr client_page_provider::get_page(int new_page_id, int old_page_id)
{
    page_request *server_msg = get_page_request(memory_msg_buffer_, memory_msg_id_);
    server_msg->new_page_id = new_page_id;
    server_msg->old_page_id = old_page_id;
    server_msg->status = state::REQUESTING;

    messenger_->poke();

    while (server_msg->status == state::REQUESTING)
    {}

    if (server_msg->status != state::WAITING)
    {
        std::string path = get_fileinfo().get_path(new_page_id);
        throw std::runtime_error("failed to request page " + path);
    }
    return local_page_provider::get_page(new_page_id, old_page_id);
}

void client_page_provider::release_page(int page_id)
{
    page_request *server_msg = get_page_request(memory_msg_buffer_, memory_msg_id_);
    std::string path = get_fileinfo().get_path(page_id);
    server_msg->new_page_id = page_id;
    server_msg->status = state::RELEASING;

    messenger_->poke();

    while (server_msg->status == state::RELEASING)
    {}

    if (server_msg->status != state::WAITING)
    {
        throw std::runtime_error("failed to release page " + path);
    }
    local_page_provider::release_page(page_id);
}

void client_page_provider::release_all()
{
    for (auto element : get_all_pages())
    {
        release_page(element.first);
    }
}
