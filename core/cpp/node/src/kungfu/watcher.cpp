//
// Created by Keren Dong on 2020/1/14.
//

#include "watcher.h"
#include "commission_store.h"
#include "config_store.h"
#include "history.h"
#include <sstream>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
inline std::string format(uint32_t uid) { return fmt::format("{:08x}", uid); }

Napi::FunctionReference Watcher::constructor;

inline location_ptr GetWatcherLocation(const Napi::CallbackInfo &info) {
  auto name = info[1].As<Napi::String>().Utf8Value();
  return std::make_shared<location>(mode::LIVE, category::SYSTEM, "node", name, IODevice::GetLocator(info));
}

Watcher::Watcher(const Napi::CallbackInfo &info)
    : ObjectWrap(info), apprentice(GetWatcherLocation(info), true),
      ledger_location_(location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", get_locator())),
      broker_client_(*this), bookkeeper_(*this, broker_client_),
      history_ref_(Napi::ObjectReference::New(History::NewInstance({info[0]}).ToObject(), 1)),
      config_ref_(Napi::ObjectReference::New(ConfigStore::NewInstance({info[0]}).ToObject(), 1)),
      commission_ref_(Napi::ObjectReference::New(CommissionStore::NewInstance({info[0]}).ToObject(), 1)),
      state_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
      ledger_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
      app_states_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)), update_state(state_ref_),
      update_ledger(ledger_ref_), publish(*this, state_ref_), reset_cache(*this, ledger_ref_) {
  serialize::InitStateMap(info, state_ref_);
  serialize::InitStateMap(info, ledger_ref_);
  SPDLOG_INFO("watcher created at {}", get_io_device()->get_home()->uname);

  auto config_store = ConfigStore::Unwrap(config_ref_.Value());

  for (const auto &app_location : config_store->profile_.get_all(Location{})) {
    add_location(0, location::make_shared(app_location, get_locator()));
  }

  auto today = time::today_start();
  for (const auto &config : config_store->profile_.get_all(Config{})) {
    auto state_location = location::make_shared(config, get_locator());
    if (state_location->category == category::STRATEGY) {
      auto mode = state_location->mode;
      auto strategy_name = state_location->name;
      auto proxy_location = location::make_shared(mode, category::STRATEGY, "node", strategy_name, get_locator());
      proxy_locations_.emplace(proxy_location->uid, proxy_location);
    }
    RestoreState(state_location, today, INT64_MAX);
  }
  RestoreState(ledger_location_, today, INT64_MAX);
  SPDLOG_INFO("watcher ledger restored");

  book::AccountingMethod::setup_defaults(bookkeeper_);
}

Watcher::~Watcher() {
  app_states_ref_.Unref();
  ledger_ref_.Unref();
  state_ref_.Unref();
  config_ref_.Unref();
  history_ref_.Unref();
}

void Watcher::NoSet(const Napi::CallbackInfo &info, const Napi::Value &value) {
  SPDLOG_WARN("do not manipulate watcher internals");
}

Napi::Value Watcher::GetLocator(const Napi::CallbackInfo &info) {
  return std::dynamic_pointer_cast<Locator>(get_locator())->get_js_locator();
}

Napi::Value Watcher::GetConfig(const Napi::CallbackInfo &info) { return config_ref_.Value(); }

Napi::Value Watcher::GetHistory(const Napi::CallbackInfo &info) { return history_ref_.Value(); }

Napi::Value Watcher::GetCommission(const Napi::CallbackInfo &info) { return commission_ref_.Value(); }

Napi::Value Watcher::GetState(const Napi::CallbackInfo &info) { return state_ref_.Value(); }

Napi::Value Watcher::GetLedger(const Napi::CallbackInfo &info) { return ledger_ref_.Value(); }

Napi::Value Watcher::GetAppStates(const Napi::CallbackInfo &info) { return app_states_ref_.Value(); }

Napi::Value Watcher::GetTradingDay(const Napi::CallbackInfo &info) {
  return Napi::String::New(ledger_ref_.Env(), time::strftime(get_trading_day(), "%Y%m%d"));
}

Napi::Value Watcher::IsUsable(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_usable()); }

Napi::Value Watcher::IsLive(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_live()); }

Napi::Value Watcher::IsStarted(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_started()); }

Napi::Value Watcher::Setup(const Napi::CallbackInfo &info) {
  try {
    setup();
    return Napi::Value();
  } catch (const std::exception &ex) {
    throw Napi::Error::New(info.Env(), fmt::format("setup failed: {}", ex.what()));
  } catch (...) {
    throw Napi::Error::New(info.Env(), "setup failed");
  }
}

Napi::Value Watcher::Step(const Napi::CallbackInfo &info) {
  try {
    step();
    return Napi::Value();
  } catch (const std::exception &ex) {
    throw Napi::Error::New(info.Env(), fmt::format("step failed: {}", ex.what()));
  } catch (...) {
    throw Napi::Error::New(info.Env(), "step failed");
  }
}

Napi::Value Watcher::GetLocation(const Napi::CallbackInfo &info) {
  auto location = FindLocation(info);
  if (not location) {
    return Napi::Value();
  }
  auto locationObj = Napi::Object::New(info.Env());
  locationObj.Set("category", Napi::String::New(info.Env(), get_category_name(location->category)));
  locationObj.Set("group", Napi::String::New(info.Env(), location->group));
  locationObj.Set("name", Napi::String::New(info.Env(), location->name));
  locationObj.Set("mode", Napi::String::New(info.Env(), get_mode_name(location->mode)));
  locationObj.Set("uname", Napi::String::New(info.Env(), location->uname));
  locationObj.Set("uid", Napi::Number::New(info.Env(), location->uid));
  locationObj.Set("locator", std::dynamic_pointer_cast<Locator>(location->locator)->get_js_locator());
  return locationObj;
}

Napi::Value Watcher::PublishState(const Napi::CallbackInfo &info) {
  publish(info[0]);
  return Napi::Value();
}

Napi::Value Watcher::isReadyToInteract(const Napi::CallbackInfo &info) {
  auto account_location = ExtractLocation(info, 0, get_locator());
  return Napi::Boolean::New(info.Env(), account_location and has_writer(account_location->uid));
}

Napi::Value Watcher::IssueOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("issue order from node");
  return InteractWithTD<OrderInput>(info, &OrderInput::order_id);
}

Napi::Value Watcher::CancelOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("cancel order from node");
  return InteractWithTD<OrderAction>(info, &OrderAction::order_action_id);
}

void Watcher::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Watcher",
                                    {
                                        InstanceMethod("isUsable", &Watcher::IsUsable),
                                        InstanceMethod("isLive", &Watcher::IsLive),
                                        InstanceMethod("isStarted", &Watcher::IsStarted),
                                        InstanceMethod("setup", &Watcher::Setup),
                                        InstanceMethod("step", &Watcher::Step),
                                        InstanceMethod("getLocation", &Watcher::GetLocation),
                                        InstanceMethod("publishState", &Watcher::PublishState),
                                        InstanceMethod("isReadyToInteract", &Watcher::isReadyToInteract),
                                        InstanceMethod("issueOrder", &Watcher::IssueOrder),
                                        InstanceMethod("cancelOrder", &Watcher::CancelOrder),
                                        InstanceAccessor("locator", &Watcher::GetLocator, &Watcher::NoSet),
                                        InstanceAccessor("config", &Watcher::GetConfig, &Watcher::NoSet),
                                        InstanceAccessor("history", &Watcher::GetHistory, &Watcher::NoSet),
                                        InstanceAccessor("commission", &Watcher::GetCommission, &Watcher::NoSet),
                                        InstanceAccessor("state", &Watcher::GetState, &Watcher::NoSet),
                                        InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::NoSet),
                                        InstanceAccessor("appStates", &Watcher::GetAppStates, &Watcher::NoSet),
                                        InstanceAccessor("tradingDay", &Watcher::GetTradingDay, &Watcher::NoSet),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Watcher", func);
}

void Watcher::on_react() {
  events_ | $([&](const event_ptr &event) { cast_event_invoke(event, update_state); });
}

void Watcher::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);

  events_ | $([&](const event_ptr &event) { cast_event_invoke(event, update_ledger); });

  events_ | is(Channel::tag) | $([&](const event_ptr &event) {
    const Channel &channel = event->data<Channel>();
    if (channel.source_id != get_home_uid() and channel.dest_id != get_home_uid()) {
      reader_->join(get_location(channel.source_id), channel.dest_id, event->gen_time());
    }
  });

  events_ | is(Register::tag) | $([&](const event_ptr &event) {
    auto register_data = event->data<Register>();
    if (register_data.location_uid == get_home_uid()) {
      return;
    }

    auto app_location = get_location(register_data.location_uid);

    if (app_location->category == category::MD or app_location->category == category::TD) {
      auto state = Napi::Number::New(app_states_ref_.Env(), (int)BrokerState::Connected);
      app_states_ref_.Set(format(app_location->uid), state);
    }

    if (app_location->category == category::MD and app_location->mode == mode::LIVE) {
      MonitorMarketData(event->gen_time(), app_location);
    }

    if (app_location->category == category::STRATEGY) {
      auto resume_time_point = broker_client_.get_resume_policy().get_connect_time(*this, register_data);
      request_write_to(event->gen_time(), app_location->uid);
      request_read_from(event->gen_time(), app_location->uid, resume_time_point);
      request_read_from_public(event->gen_time(), app_location->uid, resume_time_point);
    }
  });

  events_ | is(Deregister::tag) | $([&](const event_ptr &event) {
    auto app_location = location::make_shared(event->data<Deregister>(), get_locator());
    auto state = Napi::Number::New(app_states_ref_.Env(), (int)BrokerState::Unknown);
    app_states_ref_.Set(format(app_location->uid), state);
  });

  events_ | is(BrokerStateUpdate::tag) | $([&](const event_ptr &event) {
    auto app_location = get_location(event->source());
    auto state = Napi::Number::New(app_states_ref_.Env(), (int)(event->data<BrokerStateUpdate>().state));
    app_states_ref_.Set(format(app_location->uid), state);
  });

  events_ | is(CacheReset::tag) | $([&](const event_ptr &event) { reset_cache(event); });

  events_ | is(Quote::tag) | $([&](const event_ptr &event) { UpdateBook(event, event->data<Quote>()); });

  events_ | is(OrderInput::tag) | $([&](const event_ptr &event) { UpdateBook(event, event->data<OrderInput>()); });

  events_ | is(Order::tag) | $([&](const event_ptr &event) { UpdateBook(event, event->data<Order>()); });

  events_ | is(Trade::tag) | $([&](const event_ptr &event) { UpdateBook(event, event->data<Trade>()); });
}

void Watcher::RestoreState(const location_ptr &state_location, int64_t from, int64_t to) {
  add_location(0, state_location);
  serialize::JsRestoreState(ledger_ref_, state_location)(from, to);
}

location_ptr Watcher::FindLocation(const Napi::CallbackInfo &info) {
  if (info.Length() == 0) {
    return get_io_device()->get_home();
  }
  uint32_t uid = 0;
  if (info[0].IsNumber()) {
    uid = info[0].ToNumber().Uint32Value();
  }
  if (info[0].IsString()) {
    std::stringstream ss;
    ss << std::hex << info[0].ToString().Utf8Value();
    ss >> uid;
  }
  if (has_location(uid)) {
    return get_location(uid);
  }
  if (proxy_locations_.find(uid) != proxy_locations_.end()) {
    return proxy_locations_.at(uid);
  }
  return location_ptr();
}

void Watcher::MonitorMarketData(int64_t trigger_time, const location_ptr &md_location) {
  events_ | is(Quote::tag) | from(md_location->uid) | first() |
      $(
          [&, trigger_time, md_location](const event_ptr &event) {
            auto ready = Napi::Number::New(app_states_ref_.Env(), (int)BrokerState::Ready);
            app_states_ref_.Set(format(md_location->uid), ready);
            events_ | from(md_location->uid) | is(Quote::tag) | timeout(std::chrono::seconds(5)) |
                $(noop_event_handler(), [&, trigger_time, md_location](std::exception_ptr e) {
                  if (is_location_live(md_location->uid)) {
                    auto idle = Napi::Number::New(app_states_ref_.Env(), (int)BrokerState::Idle);
                    app_states_ref_.Set(format(md_location->uid), idle);
                    MonitorMarketData(trigger_time, md_location);
                  }
                });
          },
          error_handler_log(fmt::format("monitor md {}", md_location->uname)));
}
} // namespace kungfu::node