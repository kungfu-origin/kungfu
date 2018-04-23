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
 * IEngine: base class of all wingchun engine.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   April, 2017
 * base class of both md_engine / td_engine,
 * init reader / writer / logger
 * loop over system journal and follow its control
 */

#include "IEngine.h"
#include "longfist/sys_messages.h"
#include "TypeConvert.hpp"
#include <unistd.h>
#include <csignal>

USING_WC_NAMESPACE

#define WRITE_ENGINE_STATUS(name) \
    if (writer.get() != nullptr) \
    {\
        writer->write_frame(name, sizeof(name), source_id, MSG_TYPE_ENGINE_STATUS, 1, getpid());\
    }


volatile int IEngine::signal_received = -1;

IEngine::IEngine(short source): source_id(source), isRunning(false)
{}

IEngine::~IEngine()
{
    stop();
}

bool IEngine::start()
{
    isRunning = false;
    if (try_login())
    {
        pre_run();
        isRunning = true;
        if (reader.get() == nullptr)
        {
            throw std::runtime_error("reader is not inited! please call init() before start()");
        }
        std::signal(SIGTERM, IEngine::signal_handler);
        std::signal(SIGINT, IEngine::signal_handler);
        std::signal(SIGHUP, IEngine::signal_handler);
        std::signal(SIGQUIT, IEngine::signal_handler);
        std::signal(SIGKILL, IEngine::signal_handler);
        set_reader_thread();
    }
    return isRunning;
}

bool IEngine::stop()
{
    isRunning = false;
    if (reader_thread.get() != nullptr)
    {
        reader_thread->join();
        reader_thread.reset();
        KF_LOG_INFO(logger, "reader thread expired...");
        WRITE_ENGINE_STATUS(WC_ENGINE_STATUS_STOPPED);
        return true;
    }
    WRITE_ENGINE_STATUS(WC_ENGINE_STATUS_STOP_FAIL)
    return false;
}

bool IEngine::try_login()
{
    WRITE_ENGINE_STATUS(WC_ENGINE_STATUS_CONNECTING);
    if (!is_connected())
    {
        connect(CONNECT_TIMEOUT_NANO_SECONDS);
    }
    WRITE_ENGINE_STATUS(is_connected() ? WC_ENGINE_STATUS_CONNECTED: WC_ENGINE_STATUS_CONNECT_FAIL);
    if (is_connected() && !is_logged_in())
    {
        login(CONNECT_TIMEOUT_NANO_SECONDS);
    }
    WRITE_ENGINE_STATUS(is_logged_in() ? WC_ENGINE_STATUS_LOGIN_SUCCESS: WC_ENGINE_STATUS_LOGIN_FAIL);
    return is_logged_in();
}

void IEngine::on_engine_open()
{
    KF_LOG_INFO(logger, "[CMD] ENGINE OPEN SERVICE!");
    if (is_logged_in())
    {
        KF_LOG_INFO(logger, "already running!");
    }
    else
    {
        if (try_login())
        {
            pre_run();
            KF_LOG_INFO(logger, "restarted!");
        }
        else
        {
            KF_LOG_ERROR(logger, "failed to restart!");
        }
    }
}

void IEngine::on_engine_close()
{
    KF_LOG_DEBUG(logger, "[CMD] ENGINE CLOSE SERVICE!");
    logout();
    release_api();
    WRITE_ENGINE_STATUS(WC_ENGINE_STATUS_IDLE);
}

void IEngine::wait_for_stop()
{
    if (reader_thread.get() != nullptr)
    {
        reader_thread->join();
        reader_thread.reset();
    }
}

void IEngine::initialize(const string& json_str)
{
    // init reader / writer / logger first
    init();
    // record its status
    WRITE_ENGINE_STATUS(WC_ENGINE_STATUS_IDLE);
    // prepare config information
    json j_config = json::parse(json_str);
    for (json::const_iterator iter = j_config.begin(); iter != j_config.end(); ++iter)
    {
        KF_LOG_INFO(logger, "IEngine::init " << iter.key() << "->" << iter.value().dump());
    }
    // pre_load process is mainly for IMDEngine or ITDEngine,
    // engine-wise control information loading, sometimes needs to be loaded before.
    pre_load(j_config);
    // load config information
    load(j_config);
}

static string utf8_error_msg;

const char* EngineUtil::gbkErrorMsg2utf8(const char* errorMsg)
{
    if (errorMsg == nullptr)
    {
        return nullptr;
    }
    utf8_error_msg = kungfu::yijinjing::gbk2utf8(string(errorMsg));
    return utf8_error_msg.c_str();
}