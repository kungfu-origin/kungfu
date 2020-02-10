//
// Created by Keren Dong on 2019/12/25.
//

#include "journal.h"
#include "io.h"
#include "watcher.h"

namespace kungfu::node
{
    Napi::Object InitAll(Napi::Env env, Napi::Object exports)
    {
        Frame::Init(env, exports);
        Reader::Init(env, exports);
        Locator::Init(env, exports);
        IODevice::Init(env, exports);
        Session::Init(env, exports);
        Watcher::Init(env, exports);
        return exports;
    }

    NODE_API_MODULE(addon, InitAll)
}