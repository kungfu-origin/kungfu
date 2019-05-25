//
// Created by Keren Dong on 2019-05-25.
//

#include <spdlog/spdlog.h>
#include <nanomsg/nn.h>

#include <kungfu/yijinjing/nanomsg/socket.h>

using namespace kungfu::yijinjing::nanomsg;

const char *exception::what () const throw ()
{
    return nn_strerror (errno_);
}

int exception::num () const
{
    return errno_;
}

const char *symbol (int i, int *value)
{
    return nn_symbol (i, value);
}

void *allocmsg (size_t size, int type)
{
    void *msg = nn_allocmsg (size, type);
    if (!msg)
        throw exception ();
    return msg;
}

int freemsg (void *msg)
{
    int rc = nn_freemsg (msg);
    if (rc != 0)
        throw exception ();
    return rc;
}

void term ()
{
    nn_term ();
}

socket::socket (int domain, int protocol)
{
    sock_ = nn_socket (domain, protocol);
    if (sock_ < 0)
        throw exception ();
}

socket::~socket ()
{
    nn_close (sock_);
    //assert (rc == 0);
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

int socket::bind (const char *addr)
{
    int rc = nn_bind (sock_, addr);
    if (rc < 0)
        throw exception ();
    return rc;
}

int socket::connect (const char *addr)
{
    int rc = nn_connect (sock_, addr);
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

int socket::send (const void *buf, size_t len, int flags)
{
    int rc = nn_send (sock_, buf, len, flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return -1;
    }
    return rc;
}

int socket::recv (void *buf, size_t len, int flags)
{
    int rc = nn_recv (sock_, buf, len, flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return -1;
    }
    return rc;
}

int socket::sendmsg (const struct nn_msghdr *msghdr, int flags)
{
    int rc = nn_sendmsg (sock_, msghdr, flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return -1;
    }
    return rc;
}

int socket::recvmsg (struct nn_msghdr *msghdr, int flags)
{
    int rc = nn_recvmsg (sock_, msghdr, flags);
    if (rc < 0) {
        if (nn_errno () != EAGAIN)
            throw exception ();
        return -1;
    }
    return rc;
}
