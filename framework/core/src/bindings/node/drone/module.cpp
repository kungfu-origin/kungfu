// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2022/2/4.
//
#include <napi.h>

Napi::Object InitAll([[maybe_unused]] Napi::Env env, Napi::Object exports) { return exports; }

NODE_API_MODULE(module, InitAll)