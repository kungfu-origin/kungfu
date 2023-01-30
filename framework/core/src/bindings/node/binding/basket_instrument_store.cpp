#include "basket_instrument_store.h"
#include "io.h"

using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
Napi::FunctionReference BasketInstrumentStore::constructor = {};

BasketInstrumentStore::BasketInstrumentStore(const Napi::CallbackInfo &info)
    : ObjectWrap(info), locator_(ExtractRuntimeLocatorByInfo0(info)), profile_(locator_) {}

Napi::Value BasketInstrumentStore::SetAllBasketInstrument(const Napi::CallbackInfo &info) {
  if (info[0].IsArray()) {
    auto args = info[0].As<Napi::Array>();
    std::vector<BasketInstrument> basket_instruments;
    for (int i = 0; i < args.Length(); i++) {
      BasketInstrument basket_instrument = {};
      get(args.Get(i).ToObject(), basket_instrument);
      basket_instruments.push_back(basket_instrument);
    }
    try {
      profile_.remove_all<BasketInstrument>();
      for (auto basket : basket_instruments) {
        profile_.set(basket);
      }
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("failed to SetAllBasketInstrument {}", ex.what());
      yijinjing::util::print_stack_trace();
      return Napi::Boolean::New(info.Env(), false);
    }
  }
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value BasketInstrumentStore::GetAllBasketInstrument(const Napi::CallbackInfo &info) {
  try {
    auto basket_instruments = profile_.get_all(BasketInstrument{});
    auto result = Napi::Array::New(info.Env(), basket_instruments.size());
    for (int i = 0; i < basket_instruments.size(); i++) {
      auto target = Napi::Object::New(info.Env());
      set(basket_instruments[i], target);
      result.Set(i, target);
    }
    return result;
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("failed to GetAllBasketInstrument {}", ex.what());
    yijinjing::util::print_stack_trace();
    return Napi::Boolean::New(info.Env(), false);
  }
}

void BasketInstrumentStore::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "BasketInstrumentStore",
                  {
                      InstanceMethod("setAllBasketInstrument", &BasketInstrumentStore::SetAllBasketInstrument),
                      InstanceMethod("getAllBasketInstrument", &BasketInstrumentStore::GetAllBasketInstrument),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("BasketInstrumentStore", func);
}

Napi::Value BasketInstrumentStore::NewInstance(const Napi::Value arg) { return constructor.New({arg}); }

} // namespace kungfu::node