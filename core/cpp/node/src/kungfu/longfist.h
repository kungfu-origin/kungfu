//
// Created by Keren Dong on 2020/2/15.
//

#ifndef KUNGFU_NODE_LONGFIST_H
#define KUNGFU_NODE_LONGFIST_H

#include <napi.h>
#include <boost/hana.hpp>
#include "operators.h"

namespace kungfu::node
{
    class Longfist : public Napi::ObjectWrap<Longfist>
    {
    public:
        static void Init(Napi::Env env, Napi::Object exports);

    private:
        explicit Longfist(const Napi::CallbackInfo &info);

        template <typename DataType, const char *type_name>
        static constexpr auto make = serialize::JsMake<DataType, type_name>{};

        static Napi::ObjectReference constructor;
    };
}

#endif //KUNGFU_NODE_LONGFIST_H
