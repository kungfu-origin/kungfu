//
// Created by Keren Dong on 2022/2/4.
//
#include <napi.h>

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return exports;
}

NODE_API_MODULE(module, InitAll)