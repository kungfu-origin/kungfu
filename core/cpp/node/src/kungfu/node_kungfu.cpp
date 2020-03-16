//
// Created by Keren Dong on 2019/12/25.
//

#include <kungfu/yijinjing/io.h>

#include "longfist.h"
#include "config_store.h"
#include "commission_store.h"
#include "journal.h"
#include "io.h"
#include "watcher.h"

namespace kungfu::node
{
    Napi::Object InitAll(Napi::Env env, Napi::Object exports)
    {
        yijinjing::ensure_sqlite_initilize();

        Longfist::Init(env, exports);
        ConfigStore::Init(env, exports);
        CommissionStore::Init(env, exports);
        Frame::Init(env, exports);
        Reader::Init(env, exports);
        IODevice::Init(env, exports);
        Session::Init(env, exports);
        Watcher::Init(env, exports);

        return exports;
    }

    NODE_API_MODULE(addon, InitAll)
}
