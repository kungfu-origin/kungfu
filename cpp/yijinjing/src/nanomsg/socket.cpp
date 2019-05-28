//
// Created by Keren Dong on 2019-05-25.
//

#include <spdlog/spdlog.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>

#include <kungfu/yijinjing/nanomsg/socket.h>

using namespace kungfu::yijinjing::nanomsg;

static const char *symbol (int i, int *value)
{
    return nn_symbol (i, value);
}

static void term ()
{
    nn_term ();
}

const char *exception::what () const throw ()
{
    return nn_strerror (errno_);
}

int exception::num () const
{
    return errno_;
}

socket::socket (int domain, int protocol, int buffer_size): buf_(buffer_size)
{
    sock_ = nn_socket (domain, protocol);
    if (sock_ < 0)
        throw exception ();
}

socket::~socket ()
{
    nn_close (sock_);
}

void socket::setsockopt (int level, int option, const void *optval,
                        size_t optvallen)
{
    int rc = nn_setsockopt (sock_, level, option, optval, optvallen);
    if (rc != 0)
        throw exception ();
}

void socket::getsockopt (int level, int option, void *optval,
                        size_t *optvallen)
{
    int rc = nn_getsockopt (sock_, level, option, optval, optvallen);
    if (rc != 0)
        throw exception ();
}

int socket::bind (const std::string &url)
{
    int rc = nn_bind (sock_, url.c_str());
    if (rc < 0)
        throw exception ();
    return rc;
}

int socket::connect (const std::string &url)
{
    int rc = nn_connect (sock_, url.c_str());
    if (rc < 0)
        throw exception ();
    return rc;
}

void socket::shutdown (int how)
{
    int rc = nn_shutdown (sock_, how);
    if (rc != 0)
        throw exception ();
}

int socket::send (const std::string& msg, int flags) const
{
    int rc = nn_send (sock_, msg.c_str(), msg.length(), flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return -1;
    }
    return rc;
}

int socket::recv (int flags)
{
    int rc = nn_recv (sock_, buf_.data(), buf_.size(), flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return rc;
    }
    message_.assign(buf_.data(), rc);
    return rc;
}

int socket::send_json (const nlohmann::json &msg, int flags) const
{
    return send(msg.dump(), flags);
}

nlohmann::json socket::recv_json (int flags)
{
    if (recv(flags))
    {
        return nlohmann::json::parse(message_);
    }
    else
    {
        return nlohmann::json();
    }
}

const std::string& socket::last_message() const
{
    return message_;
}

nlohmann::json kungfu::yijinjing::nanomsg::request(const std::string &url, const nlohmann::json& msg, int timeout)
{
    socket socket(AF_SP, NN_REQ);
    socket.connect(url);
    socket.setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &timeout, sizeof(int));

    try
    {
        SPDLOG_DEBUG("request url {} msg {}", url, msg.dump());
        socket.send_json(msg);
        return socket.recv_json(0);
    }
    catch (std::exception& e)
    {
        SPDLOG_ERROR("request error: {} to {} for message {}", e.what(), url, msg.dump());
        nlohmann::json response = {};
        response["msg_type"] = msg["msg_type"];
        response["data"] = {};
        response["data"]["error_msg"] = "no response";
        return response;
    }
}
