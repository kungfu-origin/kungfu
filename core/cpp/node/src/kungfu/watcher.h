//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>

#include <kungfu/yijinjing/practice/apprentice.h>
#include "serialize.h"
#include "journal.h"

namespace kungfu::node
{
    class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice
    {
    public:
        explicit Watcher(const Napi::CallbackInfo &info);

        Napi::Value IsLive(const Napi::CallbackInfo &info);

        Napi::Value Setup(const Napi::CallbackInfo &info);

        Napi::Value Step(const Napi::CallbackInfo &info);

        Napi::Value GetLedger(const Napi::CallbackInfo &info);

        void SetLedger(const Napi::CallbackInfo &info, const Napi::Value &value);

        static void Init(Napi::Env env, Napi::Object exports);

    protected:
        void on_start() override;

    private:
        static Napi::FunctionReference constructor;
        yijinjing::data::location ledger_location_;
        Napi::ObjectReference ledger_;
        serialize::UpdateOperator ledger_update_;
    };
}


#endif //KUNGFU_NODE_WATCHER_H
