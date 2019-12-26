//
// Created by Keren Dong on 2020/1/1.
//

#ifndef KUNGFU_NODE_JOURNAL_H
#define KUNGFU_NODE_JOURNAL_H

#include <napi.h>

#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::node
{
    class Reader : public Napi::ObjectWrap<Reader>, public yijinjing::journal::reader
    {
    public:
        explicit Reader(const Napi::CallbackInfo &info);

        Napi::Value ToString(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

        static Napi::Value NewInstance();

    private:
        static Napi::FunctionReference constructor;
    };
}
#endif //KUNGFU_NODE_JOURNAL_H
