//
// Created by qlu on 2019/8/1.
//

#ifndef KUNGFU_POLLER_H
#define KUNGFU_POLLER_H

#include "os.h"
#include <functional>
#include <memory>

namespace kungfu
{
    namespace yijinjing
    {
        namespace socket
        {
            #define MAX_EVENTS 100
            #define RCV_BUFFER_SIZE 2048

            #define POLL_IN_EVENT 1
            #define POLL_OUT_EVENT 2

            typedef std::function<void(const char* data, size_t length)> rcv_handler_t;

            struct poll_event
            {
                int fd;
                uint32_t events;
                rcv_handler_t rcv_handler;
            };

            inline poll_event poll_event_in(int fd, rcv_handler_t rcv_handler)
            {
                poll_event e;
                memset(&e, 0, sizeof(poll_event));
                e.fd = fd;
                e.events |= POLL_IN_EVENT;
                e.rcv_handler = rcv_handler;
                return e;
            }

        }
    }
}

#if defined(SYS_LINUX)
#include "poller_epoll.h"
#elif defined(SYS_BSD)
#include "poller_kqueue.h"
#endif

namespace kungfu
{
    namespace yijinjing
    {
        namespace socket
        {
            typedef std::shared_ptr<poller> poller_ptr;
        }
    };
}

#endif //KUNGFU_POLLER_H
