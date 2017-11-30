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
 * Socket Handler for page engine.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 */

#include "PageSocketHandler.h"
#include "Timer.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>

USING_YJJ_NAMESPACE

using namespace boost::asio;

boost::array<char, SOCKET_MESSAGE_MAX_LENGTH> _data;
boost::shared_ptr<boost::asio::local::stream_protocol::acceptor> _acceptor;
boost::shared_ptr<local::stream_protocol::socket> _socket;
boost::shared_ptr<io_service> _io;
boost::shared_ptr<PageSocketHandler> PageSocketHandler::m_ptr = boost::shared_ptr<PageSocketHandler>(nullptr);

PageSocketHandler::PageSocketHandler(): io_running(false)
{}

PageSocketHandler* PageSocketHandler::getInstance()
{
    if (m_ptr.get() == nullptr)
    {
        m_ptr = boost::shared_ptr<PageSocketHandler>(new PageSocketHandler());
    }
    return m_ptr.get();
}

void PageSocketHandler::run(IPageSocketUtil* _util)
{
    util = _util;
    logger = util->get_logger();
    _io.reset(new io_service());
    _acceptor.reset(new local::stream_protocol::acceptor(*_io, local::stream_protocol::endpoint(PAGED_SOCKET_FILE)));
    _socket.reset(new local::stream_protocol::socket(*_io));

    _acceptor->async_accept(*_socket, boost::bind(&PageSocketHandler::handle_accept, this));
    io_running = true;
    _io->run();
}

bool PageSocketHandler::is_running()
{
    return io_running;
}

void PageSocketHandler::stop()
{
    if (_io.get() != nullptr)
        _io->stop();
    if (_socket.get() != nullptr)
        _socket->close();
    if (_acceptor.get() != nullptr)
        _acceptor->close();
    io_running = false;
}

void PageSocketHandler::handle_accept()
{
    _socket->read_some(buffer(_data));
    util->acquire_mutex();
    process_msg();
    util->release_mutex();
    _socket.reset(new local::stream_protocol::socket(*_io));
    _acceptor->async_accept(*_socket, boost::bind(&PageSocketHandler::handle_accept, this));
}

void PageSocketHandler::process_msg()
{
    PagedSocketRequest* req = (PagedSocketRequest*)&_data[0];
    byte req_type = req->type;
    KF_LOG_INFO(logger, "[socket] (status)" << (short)req_type << " ");

    switch (req_type)
    {
        case TIMER_SEC_DIFF_REQUEST:
        {
            json timer;
            timer["secDiff"] = getSecDiff();
            timer["nano"] = getNanoTime();
            strcpy(&_data[0], timer.dump().c_str());
            break;
        }
        case PAGED_SOCKET_CONNECTION_TEST:
        {
            string greetings = "Hello, world!";
            strcpy(&_data[0], greetings.c_str());
            break;
        }
        case PAGED_SOCKET_JOURNAL_REGISTER:
        {
            int idx = util->reg_journal(req->name);
            PagedSocketRspJournal rsp = {};
            rsp.type = req_type;
            rsp.success = idx >= 0;
            rsp.comm_idx = idx;
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
        case PAGED_SOCKET_STRATEGY_REGISTER:
        {
            IntPair rid_pair = util->register_strategy(req->name);
            PagedSocketRspStrategy rsp = {};
            rsp.type = req_type;
            rsp.success = rid_pair.first < rid_pair.second && rid_pair.first > 0;
            rsp.rid_start = rid_pair.first;
            rsp.rid_end = rid_pair.second;
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
        case PAGED_SOCKET_READER_REGISTER:
        case PAGED_SOCKET_WRITER_REGISTER:
        {
            string comm_file;
            int file_size;
            int has_code;
            bool ret = util->reg_client(comm_file, file_size, has_code, req->name, req->pid, req_type==PAGED_SOCKET_WRITER_REGISTER);
            PagedSocketRspClient rsp = {};
            rsp.type = req_type;
            rsp.success = ret;
            rsp.file_size = file_size;
            rsp.hash_code = has_code;
            memcpy(rsp.comm_file, comm_file.c_str(), comm_file.length() + 1);
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
        case PAGED_SOCKET_CLIENT_EXIT:
        {
            util->exit_client(req->name, req->hash_code, true);
            PagedSocketResponse rsp = {};
            rsp.type = req_type;
            rsp.success = true;
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
        case PAGED_SOCKET_SUBSCRIBE:
        case PAGED_SOCKET_SUBSCRIBE_TBC:
        {
            short source = _data[1];
            short msg_type = _data[2];
            vector<string> tickers;
            int pos = 3;
            string cur;
            while (pos < (int)SOCKET_MESSAGE_MAX_LENGTH - 1)
            {
                cur = string(&_data[pos]);
                if (cur.length() > 0)
                {
                    tickers.push_back(cur);
                    pos += cur.length() + 1;
                }
                else
                    break;
            }
            bool ret = util->sub_md(tickers, source, msg_type, req_type == PAGED_SOCKET_SUBSCRIBE);
            PagedSocketResponse rsp = {};
            rsp.type = req_type;
            rsp.success = ret;
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
        case PAGED_SOCKET_TD_LOGIN:
        {
            bool ret = util->login_td(req->name, req->source);
            PagedSocketResponse rsp = {};
            rsp.type = req_type;
            rsp.success = ret;
            memcpy(&_data[0], &rsp, sizeof(rsp));
            break;
        }
    }
    write(*_socket, buffer(_data));
}