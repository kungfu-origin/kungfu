//
// Created by qlu on 2019/8/1.
//

#ifndef KUNGFU_POLLER_EPOLL_H
#define KUNGFU_POLLER_EPOLL_H

#include "poller.h"
#include <sys/epoll.h>
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace socket
        {
            class poller
            {
            public:
                poller(): live_(false)
                {
                    epoll_fd_ = epoll_create1(0);
                    if (epoll_fd_ == -1)
                    {
                        SPDLOG_ERROR("epoll_create1 error, errno: {}, error msg: {}", errno, strerror(errno));
                    }
                }

                bool live() const { return live_; }

                void stop() { live_ = false; }

                void poll(std::vector<poll_event> events)
                {
                    for (size_t idx =0; idx < events.size(); idx ++)
                    {
                        const poll_event& poll_ev = events[idx];
                        SPDLOG_DEBUG("add event, idx: {}, fd: {}, events: {}", idx, poll_ev.fd, poll_ev.events);
                        struct epoll_event epoll_ev;
                        epoll_ev.data.u64 = idx;
                        if (poll_ev.events & POLL_IN_EVENT)
                        {
                            epoll_ev.events |= EPOLLIN;
                        }
                        if (poll_ev.events & POLL_OUT_EVENT)
                        {
                            epoll_ev.events |= EPOLLOUT;
                        }
                        int s = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, poll_ev.fd, &epoll_ev);
                        if(s == -1)
                        {
                            SPDLOG_ERROR("epoll_ctl error, epfd: {}, errno: {}, error msg: {}", epoll_fd_, errno, strerror(errno));
                        }
                    }

                    live_ = true;
                    while (live_)
                    {
                        int n = epoll_wait(epoll_fd_, epoll_events_, MAX_EVENTS, 0);
                        for (size_t idx = 0; idx < n; idx++)
                        {
                            const epoll_event& epoll_ev = epoll_events_[idx];
                            if (epoll_ev.events & EPOLLIN)
                            {
                                const poll_event& poll_ev = events.at(epoll_ev.data.u64);
                                memset(buffer_, 0, sizeof(buffer_));
                                size_t data_length = read(poll_ev.fd, buffer_, RCV_BUFFER_SIZE);
                                if (data_length > 0)
                                {
                                    poll_ev.rcv_handler(buffer_, data_length);
                                }
                            }
                        }
                    }
                }

            private:
                bool live_;
                char buffer_[RCV_BUFFER_SIZE];
                int epoll_fd_;
                struct epoll_event epoll_events_[MAX_EVENTS];

            };
        }
    }
}

#endif //KUNGFU_POLLER_EPOLL_H
