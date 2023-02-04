#include "basket_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference BasketStore::constructor = {};

BasketStore::BasketStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(ExtractRuntimeLocatorByInfo0(info)), profile_(locator_) {}

Napi::Value BasketStore::SetAllBasket(const Napi::CallbackInfo &info) {
  if (info[0].IsArray()) {
    auto args = info[0].As<Napi::Array>();
    std::vector<Basket> baskets;
    for (int i = 0; i < args.Length(); i++) {
      Basket basket = {};
      get(args.Get(i).ToObject(), basket);
      if (basket.id == (uint32_t)0) {
        basket.id = util::hash_str_32(basket.name);
      }
      baskets.push_back(basket);
    }
    try {
      profile_.remove_all<Basket>();
      for (auto basket : baskets) {
        profile_.set(basket);
      }
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("failed to SetAllBasket {}", ex.what());
      yijinjing::util::print_stack_trace();
      return Napi::Boolean::New(info.Env(), false);
    }
  }
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value BasketStore::GetAllBasket(const Napi::CallbackInfo &info) {
  try {
    auto baskets = profile_.get_all(Basket{});
    auto result = Napi::Array::New(info.Env(), baskets.size());
    for (int i = 0; i < baskets.size(); i++) {
      auto target = Napi::Object::New(info.Env());
      set(baskets[i], target);
      result.Set(i, target);
    }
    return result;
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to GetAllBasket {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }
}

void BasketStore::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "BasketStore",
                                    {
                                        InstanceMethod("setAllBasket", &BasketStore::SetAllBasket),
                                        InstanceMethod("getAllBasket", &BasketStore::GetAllBasket),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("BasketStore", func);
}

Napi::Value BasketStore::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }

} // namespace kungfu::node