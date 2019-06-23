//
// Created by Keren Dong on 2019-05-25.
//

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/util/util.h>
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

const char *nn_exception::what () const throw ()
{
    return nn_strerror (errno_);
}

int nn_exception::num () const
{
    return errno_;
}

socket::socket (int domain, protocol p, int buffer_size): protocol_(p), buf_(buffer_size)
{
    sock_ = nn_socket (domain, static_cast<int>(p));
    if (sock_ < 0)
    {
        SPDLOG_DEBUG("can not create socket");
        throw nn_exception ();
    }
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
    {
        SPDLOG_DEBUG("can not setsockopt");
        throw nn_exception ();
    }
}

void socket::setsockopt_str (int level, int option, std::string value)
{
    setsockopt(level, option, value.c_str(), value.length());
}

void socket::setsockopt_int (int level, int option, int value)
{
    setsockopt(level, option, &value, sizeof(value));
}

void socket::getsockopt (int level, int option, void *optval,
                        size_t *optvallen)
{
    int rc = nn_getsockopt (sock_, level, option, optval, optvallen);
    if (rc != 0)
    {
        SPDLOG_DEBUG("can not getsockopt");
        throw nn_exception ();
    }
}

int socket::getsockopt_int (int level, int option)
{
    int rc;
    size_t s = sizeof(rc);
    getsockopt(level, option, &rc, &s);
    return rc;
}

int socket::bind (const std::string &path)
{
    url_ = "ipc://" + path;
    relative_path_ = strip_kf_home(path);
    int rc = nn_bind (sock_, url_.c_str());
    if (rc < 0)
    {
        SPDLOG_ERROR("can not bind to {}", url_);
        throw nn_exception ();
    }
    return rc;
}

int socket::connect (const std::string &path)
{
    url_ = "ipc://" + path;
    relative_path_ = strip_kf_home(path);
    int rc = nn_connect (sock_, url_.c_str());
    if (rc < 0)
    {
        SPDLOG_ERROR("can not connect to {}", url_);
        throw nn_exception ();
    }
    return rc;
}

void socket::shutdown (int how)
{
    int rc = nn_shutdown (sock_, how);
    if (rc != 0)
    {
        SPDLOG_DEBUG("can not shutdown");
        throw nn_exception ();
    }
}

void socket::close ()
{
    int rc = nn_close (sock_);
    if (rc != 0)
    {
        SPDLOG_DEBUG("can not close");
        throw nn_exception ();
    }
}

int socket::send (const std::string& msg, int flags) const
{
    int rc = nn_send (sock_, msg.c_str(), msg.length(), flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
        {
            SPDLOG_ERROR("can not send to {} errno [{}] {}", url_, nn_errno(), nn_strerror(nn_errno()));
            throw nn_exception ();
        }
        return -1;
    }
    return rc;
}

int socket::recv (int flags)
{
    int rc = nn_recv (sock_, buf_.data(), buf_.size(), flags);
    if (rc < 0) {
        switch (nn_errno())
        {
            case ETIMEDOUT:
            case EAGAIN:
                break;
            case EINTR:
            {
                SPDLOG_WARN("interrupted when receiving from {}", relative_path_);
                throw nn_exception ();
            }
            default:
            {
                SPDLOG_ERROR("can not recv from {} errno [{}] {}", relative_path_, nn_errno(), nn_strerror(nn_errno()));
                throw nn_exception ();
            }
        }
        message_.assign(buf_.data(), 0);
        return 0;
    } else
    {
        message_.assign(buf_.data(), rc);
        return rc;
    }
}

const std::string& socket::recv_msg(int flags)
{
    recv(flags);
    return message_;
}

int socket::send_json (const nlohmann::json &msg, int flags) const
{
    return send(msg.dump(), flags);
}

nlohmann::json socket::recv_json (int flags)
{
    int rc = 0;
    if ((rc = recv(flags)) > 0)
    {
        SPDLOG_INFO("parsing json {} {}", rc, message_);
        return nlohmann::json::parse(message_);
    }
    else
    {
        return nlohmann::json();
    }
}

const std::string& socket::request(const std::string &json_message)
{
    send(json_message);
    return recv_msg();
}
