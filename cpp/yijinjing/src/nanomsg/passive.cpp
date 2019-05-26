
/**
 * Bus service and client
 * @Author kdong (keren.dong@taurus.ai)
 * @since   May, 2019
 */

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/nanomsg/passive.h>

USING_YJJ_NAMESPACE

passive::nn_socket_holder::nn_socket_holder(int protocol, std::string url): socket_(AF_SP, protocol)
{
    socket_.connect(url);
}

void passive::emitter::poke()
{
    emit(default_msg_);
}

void passive::emitter::emit(const std::string& msg)
{
    socket_.send(msg, 0);
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
        socket_.recv();
        return true;
    }
    catch(const nanomsg::exception& e)
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
    return socket_.last_message();
}
