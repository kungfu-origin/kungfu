//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_SOCKET_H
#define KUNGFU_SOCKET_H

#include <cstring>
#include <algorithm>
#include <exception>

#include <kungfu/yijinjing/comman.h>

YJJ_NAMESPACE_START

namespace nanomsg
{

    class exception : public std::exception
    {
    public:
        exception () : errno_ (nn_errno ()) {}
        virtual const char *what () const throw ();
        int num () const;

    private:
        int errno_;
    };

    const char *symbol (int i, int *value);
    void *allocmsg (size_t size, int type);
    int freemsg (void *msg);
    void term ();

    class socket
    {
    public:
        socket (int domain, int protocol);
        ~socket ();
        void setsockopt (int level, int option, const void *optval, size_t optvallen);
        void getsockopt (int level, int option, void *optval, size_t *optvallen);
        int bind (const char *addr);
        int connect (const char *addr);
        void shutdown (int how);
        int send (const void *buf, size_t len, int flags);
        int recv (void *buf, size_t len, int flags);
        int sendmsg (const struct nn_msghdr *msghdr, int flags);
        int recvmsg (struct nn_msghdr *msghdr, int flags);

    private:
        int sock_;

        /*  Prevent making copies of the socket by accident. */
        socket (const socket&);
        void operator = (const socket&);
    };
}

YJJ_NAMESPACE_END

#endif //KUNGFU_SOCKET_H
