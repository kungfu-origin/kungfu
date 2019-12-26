//
// Created by Keren Dong on 2019/12/25.
//

#include "journal.h"
#include "io.h"

namespace kungfu::node
{
    Napi::Value InitSqlite(const Napi::CallbackInfo &info)
    {
        yijinjing::io_device::init_sqlite();
        return Napi::Value();
    }

    Napi::Object InitAll(Napi::Env env, Napi::Object exports)
    {
        exports.Set(Napi::String::New(env, "init_sqlite"), Napi::Function::New(env,InitSqlite));
        Reader::Init(env, exports);
        Locator::Init(env, exports);
        IODevice::Init(env, exports);
        Session::Init(env, exports);
        return exports;
    }

    NODE_API_MODULE(addon, InitAll)
}