//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_SOCKET_H
#define KUNGFU_SOCKET_H

#include <string>
#include <cstring>
#include <algorithm>
#include <exception>
#include <nanomsg/nn.h>

#include <kungfu/yijinjing/common.h>

YJJ_NAMESPACE_START

namespace nanomsg
{
    static const char *symbol (int i, int *value);
    static void term ();

    class exception : public std::exception
    {
    public:
        exception () : errno_ (nn_errno ()) {}
        virtual const char *what () const throw ();
        int num () const;

    private:
        int errno_;
    };

    class socket
    {
    public:
        socket (int domain, int protocol);
        ~socket ();
        void setsockopt (int level, int option, const void *optval, size_t optvallen);
        void getsockopt (int level, int option, void *optval, size_t *optvallen);
        int bind (const std::string &url);
        int connect (const std::string &url);
        void shutdown (int how);
        int send (const std::string &msg, int flags) const;
        int recv (void *buf, size_t len, int flags);
        int sendmsg (const struct nn_msghdr *msghdr, int flags) const;
        int recvmsg (struct nn_msghdr *msghdr, int flags);
        void *allocmsg (size_t size, int type);
        int freemsg (void *msg);

    private:
        int sock_;

        /*  Prevent making copies of the socket by accident. */
        socket (const socket&);
        void operator = (const socket&);
    };
}

YJJ_NAMESPACE_END

#endif //KUNGFU_SOCKET_H
