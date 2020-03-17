//
// Created by Keren Dong on 2020/3/16.
//

#include "io.h"
#include "commission_store.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node
{
    Napi::FunctionReference CommissionStore::constructor;

    CommissionStore::CommissionStore(const Napi::CallbackInfo &info) :
            ObjectWrap(info),
            locator_(IODevice::GetLocator(info)),
            profile_(locator_)
    {}

    CommissionStore::~CommissionStore()
    {}

    inline Commission getCommissionFromJs(const Napi::CallbackInfo &info, const locator_ptr &locator)
    {
        Commission commission = {};
        try
        {
            if (info[0].IsString())
            {
                strcpy(commission.instrument_id, info[0].ToString().Utf8Value().c_str());
                strcpy(commission.exchange_id, info[1].ToString().Utf8Value().c_str());
            }
            if (info[0].IsObject())
            {
                serialize::JsGet{}(info[0], commission);
            }
        } catch (const std::exception &ex)
        {
            SPDLOG_ERROR("invalid argument commission: {}", ex.what());
        }
        return commission;
    }

    Napi::Value CommissionStore::SetCommission(const Napi::CallbackInfo &info)
    {
        Commission commission = getCommissionFromJs(info, locator_);
        serialize::JsGet{}(info[0], commission);
        profile_.set(commission);
        return Napi::Value();
    }

    Napi::Value CommissionStore::GetCommission(const Napi::CallbackInfo &info)
    {
        auto result = Napi::Object::New(info.Env());
        auto commission = profile_.get(getCommissionFromJs(info, locator_));
        set(commission, result);
        return result;
    }

    Napi::Value CommissionStore::GetAllCommission(const Napi::CallbackInfo &info)
    {
        auto table = Napi::Object::New(info.Env());
        for (const auto &commission : profile_.get_all(Commission{}))
        {
            std::string uid = fmt::format("{:016x}", commission.uid());
            Napi::Value value = Napi::Object::New(info.Env());
            table.Set(uid, value);
            set(commission, value);
        }
        return table;
    }

    Napi::Value CommissionStore::RemoveCommission(const Napi::CallbackInfo &info)
    {
        profile_.remove(profile_.get(getCommissionFromJs(info, locator_)));
        return Napi::Value();
    }

    void CommissionStore::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::HandleScope scope(env);

        Napi::Function func = DefineClass(env, "CommissionStore", {
                InstanceMethod("setCommission", &CommissionStore::SetCommission),
                InstanceMethod("getCommission", &CommissionStore::GetCommission),
                InstanceMethod("getAllCommission", &CommissionStore::GetAllCommission),
                InstanceMethod("removeCommission", &CommissionStore::RemoveCommission),
        });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set("CommissionStore", func);
    }

    Napi::Value CommissionStore::NewInstance(const Napi::Value arg)
    {
        return constructor.New({arg});
    }
}
