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
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/common.h>

#define MAX_MSG_LENGTH 16 * 1024

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
        socket (int domain, int protocol, int buffer_size = MAX_MSG_LENGTH);
        ~socket ();
        void                setsockopt (int level, int option, const void *optval, size_t optvallen);
        void                getsockopt (int level, int option, void *optval, size_t *optvallen);
        int                 bind (const std::string &url);
        int                 connect (const std::string &url);
        void                shutdown (int how);
        int                 send (const std::string &msg, int flags = 0) const;
        int                 recv (int flags = 0);
        int                 send_json (const nlohmann::json &msg, int flags = 0) const;
        nlohmann::json      recv_json (int flags = 0);
        const std::string&  last_message() const;

    private:
        int sock_;
        std::vector<char> buf_;
        std::string message_;

        /*  Prevent making copies of the socket by accident. */
        socket (const socket&);
        void operator = (const socket&);
    };

    nlohmann::json request(const std::string &url, const nlohmann::json& msg, int timeout = -1);
}

YJJ_NAMESPACE_END

#endif //KUNGFU_SOCKET_H
