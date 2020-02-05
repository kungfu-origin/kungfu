//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_APP_H
#define KUNGFU_NODE_APP_H

#include <napi.h>

#include <kungfu/yijinjing/practice/apprentice.h>
#include "journal.h"

namespace kungfu::node
{

    class Apprentice : public Napi::ObjectWrap<Apprentice>, public yijinjing::practice::apprentice
    {
    public:
        explicit Apprentice(const Napi::CallbackInfo &info);

        Napi::Value IsLive(const Napi::CallbackInfo &info);

        Napi::Value Setup(const Napi::CallbackInfo &info);

        Napi::Value Step(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

    private:
        static Napi::FunctionReference constructor;
    };
}


#endif //KUNGFU_NODE_APP_H
