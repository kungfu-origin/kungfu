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

/**
 * Page Provider.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * implements IPageProvider, diverge to different usage
 */

#include "Journal.h"
#include "Page.h"
#include "PageUtil.h"
#include "PageProvider.h"
#include "PageServiceMessage.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

/** send req via socket and get response in data */
void ClientPageProvider::getSocketRspOnReq(int client_request_socket, PagedSocketRequest& req, const string& name)
{
    json input_json = json{
        {"type", req.type},
        {"name", name},
        {"pid", req.pid},
        {"hash_code", req.hash_code},
        {"source", req.source}
    };
    string input = input_json.dump();
    int bytes;
    bytes = nn_send(client_request_socket, input.c_str(), input.length(), 0);
    if (bytes < 0)
    {
        SPDLOG_ERROR("nn_send");
    }
    bytes = nn_recv(client_request_socket, response_buf, SOCKET_MESSAGE_MAX_LENGTH, 0);
    if (bytes < 0)
    {
        SPDLOG_ERROR("nn_recv");
    }
    response.assign(response_buf, bytes);
}

ClientPageProvider::ClientPageProvider(const string& clientName, bool isWriting, bool reviseAllowed):
        client_name(clientName), memory_msg_buffer(nullptr)
{
    is_writer = isWriting;
    revise_allowed = is_writer || reviseAllowed;
    client_request_socket = nn_socket(AF_SP, NN_REQ);
    if (client_request_socket < 0)
    {
        SPDLOG_ERROR("Can not create client request socket");
    }
    string server_url = "ipc://" + PAGED_SOCKET_FILE;
    int rv = nn_connect(client_request_socket, server_url.c_str());
    if (rv < 0)
    {
        SPDLOG_ERROR("Can not connect client request socket to {}", server_url);
    }
    register_client();
}

void ClientPageProvider::register_client()
{
    PagedSocketRequest req = {};
    req.type = is_writer ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER;
#ifdef _WINDOWS
    req.pid = _getpid();
#else
    req.pid = getpid();
#endif
    getSocketRspOnReq(client_request_socket, req, client_name);
    SPDLOG_INFO("parse register client response {}", response);
    json rsp_json = json::parse(response);
    PagedSocketRspClient rsp;
    rsp_json.at("type").get_to(rsp.type);
    rsp_json.at("success").get_to(rsp.success);
    rsp_json.at("error_msg").get_to(rsp.error_msg);
    rsp_json.at("memory_msg_file").get_to(rsp.memory_msg_file);
    rsp_json.at("file_size").get_to(rsp.file_size);
    rsp_json.at("hash_code").get_to(rsp.hash_code);
    hash_code = rsp.hash_code;
    if (rsp.type == req.type && rsp.success)
    {
        memory_msg_buffer = PageUtil::LoadPageBuffer(string(rsp.memory_msg_file), rsp.file_size, true, false /*server lock this already*/);
    }
    else
    {
        SPDLOG_ERROR("failed to register client {}, error: {}", client_name, rsp.error_msg);
        throw std::runtime_error("cannot register client: " + client_name);
    }
}

void ClientPageProvider::exit_client()
{// send message to say good bye
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_CLIENT_EXIT;
    req.hash_code = hash_code;
    getSocketRspOnReq(client_request_socket, req, client_name);
}

int ClientPageProvider::register_journal(const string& dir, const string& jname)
{
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_JOURNAL_REGISTER;
    getSocketRspOnReq(client_request_socket, req, client_name);
    json rsp_json = json::parse(response);
    PagedSocketRspJournal rsp;
    rsp_json.at("type").get_to(rsp.type);
    rsp_json.at("success").get_to(rsp.success);
    rsp_json.at("error_msg").get_to(rsp.error_msg);
    rsp_json.at("memory_msg_idx").get_to(rsp.memory_msg_idx);
    int memory_msg_idx = -1;
    if (rsp.type == req.type && rsp.success)
        memory_msg_idx = rsp.memory_msg_idx;
    else
        throw std::runtime_error("cannot register journal: " + client_name);

    PageServiceMessage* server_msg = GET_MEMORY_MSG(memory_msg_buffer, memory_msg_idx);
    if (server_msg->status == PAGE_OCCUPIED)
    {
        memcpy(server_msg->folder, dir.c_str(), dir.length() + 1);
        memcpy(server_msg->name, jname.c_str(), jname.length() + 1);
        server_msg->is_writer = is_writer;
        server_msg->status = PAGE_HOLDING;
    }
    else
        throw std::runtime_error("server buffer is not allocated: " + client_name);

    return memory_msg_idx;
}

PagePtr ClientPageProvider::getPage(const string &dir, const string &jname, int service_id, short pageNum)
{
    PageServiceMessage* server_msg = GET_MEMORY_MSG(memory_msg_buffer, service_id);
    server_msg->page_num = pageNum;
    server_msg->status = PAGE_REQUESTING;
    while (server_msg->status == PAGE_REQUESTING) {}

    if (server_msg->status != PAGE_ALLOCATED)
    {
        if (server_msg->status == PAGE_MORE_THAN_ONE_WRITE)
            throw std::runtime_error("more than one writer is writing " + dir + " " + jname);
        else
            return PagePtr();
    }
    return Page::load(dir, jname, pageNum, revise_allowed, true);
}

void ClientPageProvider::releasePage(void* buffer, int size, int service_id)
{
    PageUtil::ReleasePageBuffer(buffer, size, true);
}

LocalPageProvider::LocalPageProvider(bool isWriting, bool reviseAllowed)
{
    is_writer = isWriting;
    revise_allowed = is_writer || reviseAllowed;
}

PagePtr LocalPageProvider::getPage(const string &dir, const string &jname, int service_id, short pageNum)
{
    return Page::load(dir, jname, pageNum, is_writer, false);
}

void LocalPageProvider::releasePage(void* buffer, int size, int service_id)
{
    PageUtil::ReleasePageBuffer(buffer, size, false);
}
