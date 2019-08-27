//
// Created by qlu on 2019/8/1.
//

#ifndef KUNGFU_POLLER_KQUEUE_H
#define KUNGFU_POLLER_KQUEUE_H

#include "poller.h"

namespace kungfu
{
    namespace yijinjing
    {
        namespace socket
        {
            class poller
            {
            public:
                poller() : live_(false) {}
                void stop() { live_ = false; }
                void poll(std::vector <poll_event> events)
                {}
                
            private:
                bool live_;
            };


        };
    }
}

#endif //KUNGFU_POLLER_KQUEUE_H
