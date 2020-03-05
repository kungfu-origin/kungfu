//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>

#include <kungfu/yijinjing/practice/apprentice.h>
#include "operators.h"
#include "journal.h"
#include "config_store.h"

namespace kungfu::node
{
    class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice
    {
    public:
        explicit Watcher(const Napi::CallbackInfo &info);
        
        ~Watcher();

        void NoSet(const Napi::CallbackInfo &info, const Napi::Value &value);

        Napi::Value GetLocator(const Napi::CallbackInfo &info);

        Napi::Value GetConfig(const Napi::CallbackInfo &info);

        Napi::Value GetState(const Napi::CallbackInfo &info);

        Napi::Value GetLedger(const Napi::CallbackInfo &info);

        Napi::Value IsUsable(const Napi::CallbackInfo &info);

        Napi::Value IsLive(const Napi::CallbackInfo &info);

        Napi::Value IsStarted(const Napi::CallbackInfo &info);

        Napi::Value Setup(const Napi::CallbackInfo &info);

        Napi::Value Step(const Napi::CallbackInfo &info);

        Napi::Value GetLocation(const Napi::CallbackInfo &info);

        Napi::Value PublishState(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

    protected:
        void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;
        void react() override;

    private:
        static Napi::FunctionReference constructor;
        yijinjing::data::location ledger_location_;
        Napi::ObjectReference config_ref_;
        Napi::ObjectReference state_ref_;
        Napi::ObjectReference ledger_ref_;
        serialize::JsUpdateState update_state;
        serialize::JsUpdateState update_ledger;
        serialize::JsPublishState publish;

        void RestoreState(const yijinjing::data::location_ptr& config_location);
    };
}


#endif //KUNGFU_NODE_WATCHER_H
