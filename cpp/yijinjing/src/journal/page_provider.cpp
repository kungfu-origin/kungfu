/*****************************************************************************
 * Copyright [2017] [taurus.ai]
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
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <nanomsg/reqrep.h>

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/service/page_service.h>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

ClientPageProvider::ClientPageProvider(const string& client_name, bool is_writer, bool revise_allowed):
        client_name_(client_name), client_request_socket_(AF_SP, NN_REQ), memory_msg_buffer_(nullptr)
{
    is_writer_ = is_writer;
    revise_allowed_ = is_writer_ || revise_allowed;
    string url = KFS_SERVICE_URL;
    client_request_socket_.connect(url.c_str());
    register_client();
    SPDLOG_TRACE("ClientPageProvider {} is_writer {} revise {}", client_name, is_writer_, revise_allowed_);
}

/** send req via socket and get response in data */
json ClientPageProvider::request(const string &path)
{
#ifdef _WINDOWS
    int pid = _getpid();
#else
    int pid = getpid();
#endif
    request_["request"] = path;
    request_["pid"] = pid;
    request_["name"] = client_name_;
    request_["writer"] = is_writer_;
    request_["hash_code"] = hash_code_;
    string request_str = request_.dump();
    int sent_bytes = client_request_socket_.send(request_str, 0);
    SPDLOG_TRACE("sent request [{}]: {}", sent_bytes, request_str);
    emitter_.poke();
    int rc = client_request_socket_.recv();
    string response_str = client_request_socket_.last_message();
    SPDLOG_TRACE("got response [{}]: {}", rc, response_str);
    return json::parse(response_str);
}

void ClientPageProvider::register_client()
{
    json response = request(is_writer_ ? "client/register/writer" : "client/register/reader");
    hash_code_ = response["hash_code"];
    if (response["success"])
    {
        std::string file = response["memory_msg_file"];
        int size = response["file_size"];
        memory_msg_buffer_ = PageUtil::LoadPageBuffer(file, size, true, false /*server lock this already*/);
    }
    else
    {
        std::string error_msg = response["error_msg"];
        SPDLOG_ERROR("failed to register client {}, error: {}", client_name_, error_msg);
        throw std::runtime_error("cannot register client: " + client_name_);
    }
}

void ClientPageProvider::exit_client()
{
    request("client/exit");
}

int ClientPageProvider::register_journal(const string& dir, const string& jname)
{
    json response = request("journal/register");
    int memory_msg_idx = -1;
    if (response["success"])
        memory_msg_idx = response["memory_msg_idx"];
    else
        throw std::runtime_error("cannot register journal: " + client_name_);

    PageServiceMessage* server_msg = GET_MEMORY_MSG(memory_msg_buffer_, memory_msg_idx);
    if (server_msg->status == PAGE_OCCUPIED)
    {
        memcpy(server_msg->folder, dir.c_str(), dir.length() + 1);
        memcpy(server_msg->name, jname.c_str(), jname.length() + 1);
        server_msg->is_writer = is_writer_;
        server_msg->status = PAGE_HOLDING;
    }
    else
        throw std::runtime_error("server buffer is not allocated: " + client_name_);

    return memory_msg_idx;
}

PagePtr ClientPageProvider::getPage(const string &dir, const string &jname, int service_id, short page_num)
{
    PageServiceMessage* server_msg = GET_MEMORY_MSG(memory_msg_buffer_, service_id);
    server_msg->page_num = page_num;
    server_msg->status = PAGE_REQUESTING;
    emitter_.poke();
    while (server_msg->status == PAGE_REQUESTING) {}

    if (server_msg->status != PAGE_ALLOCATED)
    {
        if (server_msg->status == PAGE_MORE_THAN_ONE_WRITE)
            throw std::runtime_error("more than one writer is writing " + dir + " " + jname);
        else
            return PagePtr();
    }
    return Page::load(dir, jname, page_num, revise_allowed_, true);
}

void ClientPageProvider::releasePage(void* buffer, int size, int service_id)
{
    PageUtil::ReleasePageBuffer(buffer, size, true);
}

LocalPageProvider::LocalPageProvider(bool is_writer, bool revise_allowed)
{
    is_writer_ = is_writer;
    revise_allowed_ = is_writer_ || revise_allowed;
    SPDLOG_TRACE("LocalPageProvider is_writer {} revise {}", is_writer_, revise_allowed_);
}

PagePtr LocalPageProvider::getPage(const string &dir, const string &jname, int service_id, short page_num)
{
    return Page::load(dir, jname, page_num, is_writer_, false);
}

void LocalPageProvider::releasePage(void* buffer, int size, int service_id)
{
    PageUtil::ReleasePageBuffer(buffer, size, false);
}
