
/**
 * Bus service and client
 * @Author kdong (keren.dong@taurus.ai)
 * @since   May, 2019
 */

#include "passive.h"

#include <spdlog/spdlog.h>

USING_YJJ_NAMESPACE

passive::nn_socket_holder::nn_socket_holder(int protocol, std::string url): socket_(AF_SP, protocol)
{
    socket_.connect(url.c_str());
}

void passive::emitter::poke()
{
    emit(default_msg_);
}

void passive::emitter::emit(const std::string& msg)
{
    socket_.send(msg.c_str(), msg.length(), 0);
}

passive::notice::notice(int timeout): nn_socket_holder(NN_SUB, KFS_NOTICE_URL), timeout_(timeout)
{
    socket_.setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &timeout_, sizeof(timeout_));
    socket_.setsockopt(NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
}

bool passive::notice::wait()
{
    try
    {
        int len = socket_.recv(buf_, MAC_MSG_LENGTH, 0);
        message_.assign(buf_, len);
        return true;
    }
    catch(const nn::exception& e)
    {
        if (e.num() != ETIMEDOUT)
        {
            SPDLOG_ERROR("Unexpected nanomsg error: {}", e.what());
        }
        return false;
    }
}

const std::string& passive::notice::last_message()
{
    return message_;
}
