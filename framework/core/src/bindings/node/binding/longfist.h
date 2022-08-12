//
// Created by Keren Dong on 2020/2/15.
//

#ifndef KUNGFU_NODE_LONGFIST_H
#define KUNGFU_NODE_LONGFIST_H

#include "common.h"
#include "operators.h"

namespace kungfu::node {
class Longfist : public Napi::ObjectWrap<Longfist> {
public:
  static void Init(Napi::Env env, Napi::Object exports);

private:
  explicit Longfist(const Napi::CallbackInfo &info);

  static Napi::ObjectReference constructor;
};
} // namespace kungfu::node

#endif // KUNGFU_NODE_LONGFIST_H
