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

#include "PageProvider.h"
#include "PageCommStruct.h"
#include "PageSocketStruct.h"
#include "PageUtil.h"
#include "Page.h"
#include "StrategySocketHandler.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>

USING_YJJ_NAMESPACE

typedef boost::array<char, SOCKET_MESSAGE_MAX_LENGTH> SocketMArray;

/** get socket response via paged_socket */
void getSocketRsp(SocketMArray &input, SocketMArray &output)
{
    using namespace boost::asio;
    io_service io_service;
    local::stream_protocol::socket socket(io_service);
    socket.connect(local::stream_protocol::endpoint(PAGED_SOCKET_FILE));
    boost::system::error_code error;
    write(socket, buffer(input), error);
    socket.read_some(buffer(output), error);
}

/** send req via socket and get response in data */
void getSocketRspOnReq(PagedSocketRequest& req, SocketMArray& data, const string& name)
{
    memcpy(req.name, name.c_str(), name.length() + 1);
    SocketMArray reqBuf;
    memcpy(&reqBuf[0], &req, sizeof(req));
    getSocketRsp(reqBuf, data);
}

ClientPageProvider::ClientPageProvider(const string& clientName, bool isWriting, bool reviseAllowed):
        client_name(clientName), comm_buffer(nullptr)
{
    is_writer = isWriting;
    revise_allowed = is_writer || reviseAllowed;
    register_client();
}

void ClientPageProvider::register_client()
{
    PagedSocketRequest req = {};
    req.type = is_writer ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER;
    req.pid = getpid();
    SocketMArray rspArray;
    getSocketRspOnReq(req, rspArray, client_name);
    PagedSocketRspClient* rsp = (PagedSocketRspClient*)(&rspArray[0]);
    hash_code = rsp->hash_code;
    if (rsp->type == req.type && rsp->success)
    {
        comm_buffer = PageUtil::LoadPageBuffer(string(rsp->comm_file), rsp->file_size, true, false /*server lock this already*/);
    }
    else
    {
        throw std::runtime_error("cannot register client: " + client_name);
    }
}

void ClientPageProvider::exit_client()
{// send message to say good bye
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_CLIENT_EXIT;
    req.hash_code = hash_code;
    SocketMArray rspArray;
    getSocketRspOnReq(req, rspArray, client_name);
}

int ClientPageProvider::register_journal(const string& dir, const string& jname)
{
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_JOURNAL_REGISTER;
    SocketMArray rspArray;
    getSocketRspOnReq(req, rspArray, client_name);
    PagedSocketRspJournal* rsp = (PagedSocketRspJournal*)(&rspArray[0]);
    int comm_idx = -1;
    if (rsp->type == req.type && rsp->success)
        comm_idx = rsp->comm_idx;
    else
        throw std::runtime_error("cannot register journal: " + client_name);

    PageCommMsg* serverMsg = GET_COMM_MSG(comm_buffer, comm_idx);
    if (serverMsg->status == PAGED_COMM_OCCUPIED)
    {
        memcpy(serverMsg->folder, dir.c_str(), dir.length() + 1);
        memcpy(serverMsg->name, jname.c_str(), jname.length() + 1);
        serverMsg->is_writer = is_writer;
        serverMsg->status = PAGED_COMM_HOLDING;
    }
    else
        throw std::runtime_error("server buffer is not allocated: " + client_name);

    return comm_idx;
}

PagePtr ClientPageProvider::getPage(const string &dir, const string &jname, int serviceIdx, short pageNum)
{
    PageCommMsg* serverMsg = GET_COMM_MSG(comm_buffer, serviceIdx);
    serverMsg->page_num = pageNum;
    serverMsg->status = PAGED_COMM_REQUESTING;
    while (serverMsg->status == PAGED_COMM_REQUESTING) {}

    if (serverMsg->status != PAGED_COMM_ALLOCATED)
    {
        if (serverMsg->status == PAGED_COMM_MORE_THAN_ONE_WRITE)
            throw std::runtime_error("more than one writer is writing " + dir + " " + jname);
        else
            return PagePtr();
    }
    return Page::load(dir, jname, pageNum, revise_allowed, true);
}

void ClientPageProvider::releasePage(void* buffer, int size, int serviceIdx)
{
    PageUtil::ReleasePageBuffer(buffer, size, true);
}

LocalPageProvider::LocalPageProvider(bool isWriting, bool reviseAllowed)
{
    is_writer = isWriting;
    revise_allowed = is_writer || reviseAllowed;
}

PagePtr LocalPageProvider::getPage(const string &dir, const string &jname, int serviceIdx, short pageNum)
{
    return Page::load(dir, jname, pageNum, is_writer, false);
}

void LocalPageProvider::releasePage(void* buffer, int size, int serviceIdx)
{
    PageUtil::ReleasePageBuffer(buffer, size, false);
}

StrategySocketHandler::StrategySocketHandler(const string& strategyName):
        ClientPageProvider(strategyName, true)
{}

bool StrategySocketHandler::td_connect(short source)
{
    SocketMArray rspArray;
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_TD_LOGIN;
    req.source = source;
    getSocketRspOnReq(req, rspArray, client_name);
    PagedSocketResponse* rsp = (PagedSocketResponse*)&rspArray[0];
    return rsp->type == req.type && rsp->success;
}

bool StrategySocketHandler::md_subscribe(const vector<string>& tickers, short source, short msg_type)
{
    size_t idx = 0;
    SocketMArray reqArray;
    memset(&reqArray[0], '\0', SOCKET_MESSAGE_MAX_LENGTH);
    int pos = 3; // from pos1
    SocketMArray rspArray;
    while (idx < tickers.size())
    {
        const string& ticker = tickers[idx];
        int tickerLen = ticker.length() + 1;
        if (pos + tickerLen < SOCKET_MESSAGE_MAX_LENGTH - 1)
        {
            memcpy(&reqArray[pos], ticker.c_str(), tickerLen);
            pos += tickerLen;
            idx += 1;
        }
        else
        {
            reqArray[0] = PAGED_SOCKET_SUBSCRIBE_TBC;
            reqArray[1] = source;
            reqArray[2] = msg_type;
            getSocketRsp(reqArray, rspArray);
            PagedSocketResponse* rsp = (PagedSocketResponse*)&rspArray[0];
            if (rsp->type != reqArray[0] || !rsp->success)
                return false;
            memset(&reqArray[0], '\0', SOCKET_MESSAGE_MAX_LENGTH);
            pos = 3;
        }
    }
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE;
    reqArray[1] = source;
    reqArray[2] = msg_type;
    getSocketRsp(reqArray, rspArray);
    PagedSocketResponse* rsp = (PagedSocketResponse*)&rspArray[0];
    return rsp->type == reqArray[0] && rsp->success;
}

bool StrategySocketHandler::register_strategy(int& requestIdStart, int& requestIdEnd)
{
    PagedSocketRequest req = {};
    req.type = PAGED_SOCKET_STRATEGY_REGISTER;
    SocketMArray rspArray;
    getSocketRspOnReq(req, rspArray, client_name);
    PagedSocketRspStrategy* rsp = (PagedSocketRspStrategy*)(&rspArray[0]);
    requestIdStart = rsp->rid_start;
    requestIdEnd = rsp->rid_end;
    return rsp->type == req.type && rsp->success;
}