//
// Created by Keren Dong on 2020/3/17.
//

#ifndef KUNGFU_NODE_HISTORY_H
#define KUNGFU_NODE_HISTORY_H

#include <napi.h>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/practice/profile.h>

namespace kungfu::node
{
    class History : public Napi::ObjectWrap<History>
    {
    public:
        explicit History(const Napi::CallbackInfo &info);

        ~History() = default;

        static void Init(Napi::Env env, Napi::Object exports);

        Napi::Value SelectPeriod(const Napi::CallbackInfo &info);

        static Napi::Value NewInstance(const Napi::Value arg);

    private:
        yijinjing::data::locator_ptr locator_;
        yijinjing::data::location_ptr ledger_location_;
        yijinjing::practice::profile profile_;
        static Napi::FunctionReference constructor;
    };
}

#endif //KUNGFU_NODE_HISTORY_H
