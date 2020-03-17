//
// Created by Keren Dong on 2020/3/16.
//

#ifndef KUNGFU_NODE_COMMISSION_H
#define KUNGFU_NODE_COMMISSION_H

#include <napi.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/profile.h>

#include "operators.h"

namespace kungfu::node
{
    class CommissionStore : public Napi::ObjectWrap<CommissionStore>
    {
    public:
        explicit CommissionStore(const Napi::CallbackInfo &info);

        ~CommissionStore();

        Napi::Value SetCommission(const Napi::CallbackInfo &info);

        Napi::Value GetCommission(const Napi::CallbackInfo &info);

        Napi::Value SetAllCommission(const Napi::CallbackInfo &info);

        Napi::Value GetAllCommission(const Napi::CallbackInfo &info);

        Napi::Value RemoveCommission(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

        static Napi::Value NewInstance(const Napi::Value arg);

    private:
        serialize::JsSet set;
        yijinjing::data::locator_ptr locator_;
        yijinjing::practice::profile profile_;

        static Napi::FunctionReference constructor;

        friend class Watcher;
    };
}

#endif //KUNGFU_NODE_COMMISSION_H
