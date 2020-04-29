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

Napi::FunctionReference Watcher::constructor = {};

inline location_ptr GetWatcherLocation(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 1, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid location argument");
  }
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
  log::copy_log_settings(get_home(), get_home()->name);

  serialize::InitStateMap(info, state_ref_, "state");
  serialize::InitStateMap(info, ledger_ref_, "ledger");

  auto today = time::today_start();
  auto config_store = ConfigStore::Unwrap(config_ref_.Value());
  for (const auto &item : config_store->profile_.get_all(Location{})) {
    auto saved_location = location::make_shared(item, get_locator());
    add_location(now(), saved_location);
    RestoreState(saved_location, today, INT64_MAX);
  }
  RestoreState(ledger_location_, today, INT64_MAX);

  shift(ledger_location_) >> state_bank_; // Load positions to restore bookkeeper

  SPDLOG_INFO("watcher {} initialized", get_io_device()->get_home()->uname);
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
  return Napi::String::New(ledger_ref_.Env(), time::strftime(get_trading_day(), KUNGFU_TRADING_DAY_FORMAT));
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

Napi::Value Watcher::RequestStop(const Napi::CallbackInfo &info) {
  auto app_location = ExtractLocation(info, 0, get_locator());
  get_writer(app_location->uid)->mark(now(), RequestStop::tag);
  return Napi::Value();
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
  if (IsValid(info, 0, &Napi::Value::IsObject)) {
    publish(info[0].ToObject());
  }
  return Napi::Value();
}

Napi::Value Watcher::IsReadyToInteract(const Napi::CallbackInfo &info) {
  auto account_location = ExtractLocation(info, 0, get_locator());
  return Napi::Boolean::New(info.Env(), account_location and has_writer(account_location->uid));
}

Napi::Value Watcher::IssueOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("issue order manually");
  return InteractWithTD<OrderInput>(info, &OrderInput::order_id);
}

Napi::Value Watcher::CancelOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("cancel order manually");
  return InteractWithTD<OrderAction>(info, &OrderAction::order_action_id);
}

void Watcher::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Watcher",
                                    {
                                        InstanceMethod("isUsable", &Watcher::IsUsable),                           //
                                        InstanceMethod("isLive", &Watcher::IsLive),                               //
                                        InstanceMethod("isStarted", &Watcher::IsStarted),                         //
                                        InstanceMethod("setup", &Watcher::Setup),                                 //
                                        InstanceMethod("step", &Watcher::Step),                                   //
                                        InstanceMethod("requestStop", &Watcher::RequestStop),                     //
                                        InstanceMethod("getLocation", &Watcher::GetLocation),                     //
                                        InstanceMethod("publishState", &Watcher::PublishState),                   //
                                        InstanceMethod("isReadyToInteract", &Watcher::IsReadyToInteract),         //
                                        InstanceMethod("issueOrder", &Watcher::IssueOrder),                       //
                                        InstanceMethod("cancelOrder", &Watcher::CancelOrder),                     //
                                        InstanceAccessor("locator", &Watcher::GetLocator, &Watcher::NoSet),       //
                                        InstanceAccessor("config", &Watcher::GetConfig, &Watcher::NoSet),         //
                                        InstanceAccessor("history", &Watcher::GetHistory, &Watcher::NoSet),       //
                                        InstanceAccessor("commission", &Watcher::GetCommission, &Watcher::NoSet), //
                                        InstanceAccessor("state", &Watcher::GetState, &Watcher::NoSet),           //
                                        InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::NoSet),         //
                                        InstanceAccessor("appStates", &Watcher::GetAppStates, &Watcher::NoSet),   //
                                        InstanceAccessor("tradingDay", &Watcher::GetTradingDay, &Watcher::NoSet), //
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Watcher", func);
}

void Watcher::on_react() {
  events_ | $([&](const event_ptr &event) { feed_state_data(event, update_state); });
}

void Watcher::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);

  events_ | $$(feed_state_data(event, update_ledger));
  events_ | is(Quote::tag) | $$(UpdateBook(event, event->data<Quote>()));
  events_ | is(OrderInput::tag) | $$(UpdateBook(event, event->data<OrderInput>()));
  events_ | is(Order::tag) | $$(UpdateBook(event, event->data<Order>()));
  events_ | is(Trade::tag) | $$(UpdateBook(event, event->data<Trade>()));
  events_ | is(Position::tag) | $$(UpdateBook(event, event->data<Position>()););
  events_ | is(PositionEnd::tag) | $$(UpdateAsset(event, event->data<PositionEnd>().holder_uid););
  events_ | is(Channel::tag) | $$(InspectChannel(event->gen_time(), event->data<Channel>()));
  events_ | is(Register::tag) | $$(OnRegister(event->gen_time(), event->data<Register>()));
  events_ | is(Deregister::tag) | $$(OnDeregister(event->gen_time(), event->data<Deregister>()));
  events_ | is(BrokerStateUpdate::tag) | $$(UpdateBrokerState(event->source(), event->data<BrokerStateUpdate>()));
  events_ | is(CacheReset::tag) | $$(reset_cache(event));
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
  return location_ptr();
}

void Watcher::InspectChannel(int64_t trigger_time, const Channel &channel) {
  if (channel.source_id != get_live_home_uid() and channel.dest_id != get_live_home_uid()) {
    reader_->join(get_location(channel.source_id), channel.dest_id, trigger_time);
  }
}

void Watcher::MonitorMarketData(int64_t trigger_time, const location_ptr &md_location) {
  events_ | is(Quote::tag) | from(md_location->uid) | first() |
      $(
          [&, trigger_time, md_location](const event_ptr &event) {
            auto ready = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Ready));
            app_states_ref_.Set(format(md_location->uid), ready);
            events_ | from(md_location->uid) | is(Quote::tag) | timeout(std::chrono::seconds(5)) |
                $(noop_event_handler(), [&, trigger_time, md_location](std::exception_ptr e) {
                  if (is_location_live(md_location->uid)) {
                    auto idle = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Idle));
                    app_states_ref_.Set(format(md_location->uid), idle);
                    MonitorMarketData(trigger_time, md_location);
                  }
                });
          },
          error_handler_log(fmt::format("monitor md {}", md_location->uname)));
}

void Watcher::OnRegister(int64_t trigger_time, const Register &register_data) {
  if (register_data.location_uid == get_home_uid()) {
    return;
  }

  auto app_location = get_location(register_data.location_uid);

  if (app_location->category == category::MD or app_location->category == category::TD) {
    auto state = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Connected));
    app_states_ref_.Set(format(app_location->uid), state);
  }

  if (app_location->category == category::MD and app_location->mode == mode::LIVE) {
    MonitorMarketData(trigger_time, app_location);
  }
}

void Watcher::OnDeregister(int64_t trigger_time, const Deregister &deregister_data) {
  auto app_location = location::make_shared(deregister_data, get_locator());
  auto state = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Pending));
  app_states_ref_.Set(format(app_location->uid), state);
}

void Watcher::UpdateBrokerState(uint32_t broker_uid, const BrokerStateUpdate &state) {
  auto app_location = get_location(broker_uid);
  auto state_value = Napi::Number::New(app_states_ref_.Env(), int(state.state));
  app_states_ref_.Set(format(app_location->uid), state_value);
}

void Watcher::UpdateAsset(const event_ptr &event, uint32_t book_uid) {
  auto book = bookkeeper_.get_book(book_uid);
  book->update(event->gen_time());
  update_ledger(event->gen_time(), ledger_location_->uid, book_uid, book->asset);
}

void Watcher::UpdateBook(const event_ptr &event, const Quote &quote) {
  auto ledger_uid = ledger_location_->uid;
  for (const auto &item : bookkeeper_.get_books()) {
    auto &book = item.second;
    auto holder_uid = book->asset.holder_uid;
    if (holder_uid == ledger_uid) {
      continue;
    }
    if (book->has_long_position_for(quote)) {
      UpdateBook(event, book->get_position_for(Direction::Long, quote));
    }
    if (book->has_short_position_for(quote)) {
      UpdateBook(event, book->get_position_for(Direction::Short, quote));
    }
    update_ledger(event->gen_time(), ledger_uid, holder_uid, book->asset);
  }
}

void Watcher::UpdateBook(const event_ptr &event, const Position &position) {
  auto book = bookkeeper_.get_book(position.holder_uid);
  auto &book_position = book->get_position_for(position.direction, position);
  if (book_position.volume > 0) {
    update_ledger(event->gen_time(), event->source(), event->dest(), book_position);
  }
}
} // namespace kungfu::node