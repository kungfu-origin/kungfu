//
// Created by Keren Dong on 2019/12/25.
//

#include "longfist.h"
#include "journal.h"
#include "io.h"
#include "watcher.h"

namespace kungfu::node
{
    Napi::Object InitAll(Napi::Env env, Napi::Object exports)
    {
        Longfist::Init(env, exports);
        Frame::Init(env, exports);
        Reader::Init(env, exports);
        IODevice::Init(env, exports);
        Session::Init(env, exports);
        Watcher::Init(env, exports);
        return exports;
    }

    NODE_API_MODULE(addon, InitAll)
}