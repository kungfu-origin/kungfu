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
 * Timer for kungfu system.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide basic nano time and time transformation
 */

#include "nlohmann/json.hpp"

#include "Timer.h"
#include "Journal.h"
#include "PageSocketStruct.h"
#include <chrono>
#include <boost/asio.hpp>
#include <boost/array.hpp>

USING_YJJ_NAMESPACE

using json = nlohmann::json;

boost::shared_ptr<NanoTimer> NanoTimer::m_ptr = boost::shared_ptr<NanoTimer>(nullptr);

NanoTimer* NanoTimer::getInstance()
{
    if (m_ptr.get() == nullptr)
    {
        m_ptr = boost::shared_ptr<NanoTimer>(new NanoTimer());
    }
    return m_ptr.get();
}

inline std::chrono::steady_clock::time_point get_time_now()
{
    return std::chrono::steady_clock::now();
}

inline int64_t get_socket_diff()
{
    using namespace boost::asio;
    boost::array<char, SOCKET_MESSAGE_MAX_LENGTH> input, output;
    io_service io_service;

#ifdef _WINDOWS
    ip::tcp::socket socket(io_service);
    socket.connect(ip::tcp::endpoint(ip::tcp::v4(), PAGED_SOCKET_PORT));
#else
    local::stream_protocol::socket socket(io_service);
    socket.connect(local::stream_protocol::endpoint(PAGED_SOCKET_FILE));
#endif // _WINDOWS

    boost::system::error_code error;
    input[0] = TIMER_SEC_DIFF_REQUEST;
    write(socket, buffer(input), error);
    socket.read_some(buffer(output), error);
    json socket_info = json::parse(string(&output[0]));
    return socket_info["secDiff"].get<int64_t>();
}

inline int64_t get_local_diff()
{
    int unix_second_num = std::chrono::seconds(time(NULL)).count();
    int tick_second_num = std::chrono::duration_cast<std::chrono::seconds>(
            get_time_now().time_since_epoch()
    ).count();
    return (unix_second_num - tick_second_num) * NANOSECONDS_PER_SECOND;
}

NanoTimer::NanoTimer()
{
    try
    {
        secDiff = get_socket_diff();
    }
    catch(...)
    {
        secDiff = get_local_diff();
    }
}

int64_t NanoTimer::getNano() const
{
    auto _nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            get_time_now().time_since_epoch()
    ).count();
    return _nano + secDiff;
}
