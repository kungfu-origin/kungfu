//
// Created by qlu on 2019/1/22.
//

#include <kungfu/wingchun/event_loop/event_loop.h>

using namespace kungfu;

int main()
{
    EventLoop loop("test");
    loop.subscribe_nanomsg("ipc:///tmp/kungfu.paged.ipc");
    loop.bind_nanomsg("ipc:///tmp/reqrep.ipc");
    loop.run();
}