//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>

#include <kungfu/yijinjing/practice/apprentice.h>
#include "operators.h"
#include "journal.h"

namespace kungfu::node
{
    class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice
    {
    public:
        explicit Watcher(const Napi::CallbackInfo &info);
        
        ~Watcher();

        Napi::Value IsLive(const Napi::CallbackInfo &info);

        Napi::Value Setup(const Napi::CallbackInfo &info);

        Napi::Value Step(const Napi::CallbackInfo &info);

        Napi::Value GetLedger(const Napi::CallbackInfo &info);

        void SetLedger(const Napi::CallbackInfo &info, const Napi::Value &value);

        static void Init(Napi::Env env, Napi::Object exports);

    protected:
        void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;
        void react() override;

    private:
        static Napi::FunctionReference constructor;
        Napi::ObjectReference ledger_ref_;
        yijinjing::data::location ledger_location_;
        serialize::UpdateOperator update_ledger;
    };
}


#endif //KUNGFU_NODE_WATCHER_H
