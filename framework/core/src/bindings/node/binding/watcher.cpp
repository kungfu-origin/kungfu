//
// Created by Keren Dong on 2020/1/14.
//

#include "watcher.h"
#include "commission_store.h"
#include "config_store.h"
#include "history.h"
#include <sstream>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
  uv_loop_t *loop;
  uv_loop_t *loop_main;
  uv_work_t greq;
  uv_timer_t timer_req;
inline std::string format(uint32_t uid) { return fmt::format("{:08x}", uid); }

Napi::FunctionReference Watcher::constructor = {};

inline location_ptr GetWatcherLocation(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 1, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid location argument");
  }
  auto name = info[1].As<Napi::String>().Utf8Value();
  return std::make_shared<location>(mode::LIVE, category::SYSTEM, "node", name, std::make_shared<locator>());
}

inline bool GetBypassQuotes(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 2, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid bypassQuotes argument");
  }
  return info[2].As<Napi::Boolean>().Value();
}

inline bool GetBypassRestore(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 3, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid bypassRestore argument");
  }
  return info[3].As<Napi::Boolean>().Value();
}

Watcher::Watcher(const Napi::CallbackInfo &info)
    : ObjectWrap(info), apprentice(GetWatcherLocation(info), true),
      ledger_location_(location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", get_locator())),
      bypass_quotes_(GetBypassQuotes(info)), broker_client_(*this), bookkeeper_(*this, broker_client_),
      history_ref_(Napi::ObjectReference::New(History::NewInstance({info[0]}).ToObject(), 1)),
      config_ref_(Napi::ObjectReference::New(ConfigStore::NewInstance({info[0]}).ToObject(), 1)),
      commission_ref_(Napi::ObjectReference::New(CommissionStore::NewInstance({info[0]}).ToObject(), 1)),
      state_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
      ledger_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),
      app_states_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)), update_state(state_ref_),
      update_ledger(ledger_ref_), publish(*this, state_ref_), reset_cache(*this, ledger_ref_), start_(true) {
  log::copy_log_settings(get_home(), get_home()->name);

  serialize::InitStateMap(info, state_ref_, "state");
  serialize::InitStateMap(info, ledger_ref_, "ledger");

  auto today = time::today_start();
  auto config_store = ConfigStore::Unwrap(config_ref_.Value());

  bool sync_schema = not get_io_device()->is_usable();
  if (sync_schema) {
    config_store->profile_.setup();
  }

  if (GetBypassRestore(info)) {
    return;
  }

  for (const auto &item : config_store->profile_.get_all(Location{})) {
    auto saved_location = location::make_shared(item, get_locator());
    add_location(now(), saved_location);
    RestoreState(saved_location, today, INT64_MAX, sync_schema);
    SPDLOG_WARN("restored data for {}", saved_location->uname);
  }
  RestoreState(ledger_location_, today, INT64_MAX, sync_schema);

  shift(ledger_location_) >> state_bank_; // Load positions to restore bookkeeper
  uv_mutex_init(&mutex);
  SPDLOG_INFO("watcher {} initialized", get_io_device()->get_home()->uname);
}

Watcher::~Watcher() {
  start_ = false;
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

Napi::Value Watcher::GetLocation(const Napi::CallbackInfo &info) {
  auto location = FindLocation(info);
  if (not location) {
    return {};
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

Napi::Value Watcher::GetLocationUID(const Napi::CallbackInfo &info) {
  auto target_location = ExtractLocation(info, 0, get_locator());
  return Napi::Number::New(info.Env(), target_location->uid);
}

Napi::Value Watcher::GetInstrumentUID(const Napi::CallbackInfo &info) {
  auto exchange_id = info[0].ToString().Utf8Value();
  auto instrument_id = info[1].ToString().Utf8Value();
  auto key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  return Napi::Number::New(info.Env(), key);
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

Napi::Value Watcher::Now(const Napi::CallbackInfo &info) {
  return Napi::BigInt::New(ledger_ref_.Env(), time::now_in_nano());
}

Napi::Value Watcher::IsUsable(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_usable()); }

Napi::Value Watcher::IsLive(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_live()); }

Napi::Value Watcher::IsStarted(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_started()); }

Napi::Value Watcher::Setup(const Napi::CallbackInfo &info) {
  try {
    setup();
    return {};
  } catch (const std::exception &ex) {
    throw Napi::Error::New(info.Env(), fmt::format("setup failed: {}", ex.what()));
  } catch (...) {
    throw Napi::Error::New(info.Env(), "setup failed");
  }
}

Napi::Value Watcher::Step(const Napi::CallbackInfo &info) {
  try {
    step();
    return {};
  } catch (const std::exception &ex) {
    throw Napi::Error::New(info.Env(), fmt::format("step failed: {}", ex.what()));
  } catch (...) {
    throw Napi::Error::New(info.Env(), "step failed");
  }
}

Napi::Value Watcher::RequestStop(const Napi::CallbackInfo &info) {
  auto app_location = ExtractLocation(info, 0, get_locator());
  get_writer(app_location->uid)->mark(now(), RequestStop::tag);
  return {};
}

Napi::Value Watcher::PublishState(const Napi::CallbackInfo &info) {
  if (IsValid(info, 0, &Napi::Value::IsObject)) {
    publish(info[0].ToObject());
  }
  return {};
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

Napi::Value Watcher::RequestMarketData(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsObject)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  if (not IsValid(info, 1, &Napi::Value::IsString)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  if (not IsValid(info, 2, &Napi::Value::IsString)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  auto md_location = ExtractLocation(info, 0, get_locator());
  auto exchange_id = info[1].ToString().Utf8Value();
  auto instrument_id = info[2].ToString().Utf8Value();

  SPDLOG_INFO("request market data {}@{} from {}", exchange_id, instrument_id, md_location->uname);

  if (not has_writer(md_location->uid)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  auto writer = get_writer(md_location->uid);
  uint32_t key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  InstrumentKey instrument_key = {};
  instrument_key.key = key;
  strcpy(instrument_key.instrument_id, instrument_id.c_str());
  strcpy(instrument_key.exchange_id, exchange_id.c_str());
  instrument_key.instrument_type = get_instrument_type(exchange_id, instrument_id);
  writer->write(now(), instrument_key);
  subscribed_instruments_.emplace(key, instrument_key);

  return Napi::Boolean::New(info.Env(), true);
}

void Watcher::UpdateQuote(const Napi::CallbackInfo &info) {
  upQuote();
}

void Watcher::upQuote() {
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    UpdateLedger(+boost::hana::second(it));
  });
}

void Watcher::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Watcher",
                                    {
                                        InstanceMethod("now", &Watcher::Now),                                     //
                                        InstanceMethod("isUsable", &Watcher::IsUsable),                           //
                                        InstanceMethod("isLive", &Watcher::IsLive),                               //
                                        InstanceMethod("isStarted", &Watcher::IsStarted),                         //
                                        InstanceMethod("setup", &Watcher::Setup),                                 //
                                        InstanceMethod("step", &Watcher::Step),                                   //
                                        InstanceMethod("requestStop", &Watcher::RequestStop),                     //
                                        InstanceMethod("getLocation", &Watcher::GetLocation),                     //
                                        InstanceMethod("getLocationUID", &Watcher::GetLocationUID),               //
                                        InstanceMethod("getInstrumentUID", &Watcher::GetInstrumentUID),           //
                                        InstanceMethod("publishState", &Watcher::PublishState),                   //
                                        InstanceMethod("isReadyToInteract", &Watcher::IsReadyToInteract),         //
                                        InstanceMethod("issueOrder", &Watcher::IssueOrder),                       //
                                        InstanceMethod("updateQuote", &Watcher::UpdateQuote),                     //
                                        InstanceMethod("cancelOrder", &Watcher::CancelOrder),                     //
                                        InstanceMethod("requestMarketData", &Watcher::RequestMarketData),         //
                                        InstanceAccessor("locator", &Watcher::GetLocator, &Watcher::NoSet),       //
                                        InstanceAccessor("config", &Watcher::GetConfig, &Watcher::NoSet),         //
                                        InstanceAccessor("history", &Watcher::GetHistory, &Watcher::NoSet),       //
                                        InstanceAccessor("commission", &Watcher::GetCommission, &Watcher::NoSet), //
                                        InstanceAccessor("state", &Watcher::GetState, &Watcher::NoSet),           //
                                        InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::NoSet),         //
                                        InstanceAccessor("appStates", &Watcher::GetAppStates, &Watcher::NoSet),   //
                                        InstanceAccessor("tradingDay", &Watcher::GetTradingDay, &Watcher::NoSet), //
                                        InstanceMethod("createTask", &Watcher::CreateTask),
                                        InstanceMethod("lock", &Watcher::Lock),                                     //
                                        InstanceMethod("unlock", &Watcher::Unlock),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Watcher", func);
}

void Watcher::on_react() {
  events_ | $([&](const event_ptr &event) { feed_state_data(event, data_bank_); });
}

void Watcher::on_start() {
  broker_client_.on_start(events_);
  bookkeeper_.on_start(events_);
  bookkeeper_.guard_positions();

  events_ | bypass(this, bypass_quotes_) | $$(Feed(event));
  events_ | is(OrderInput::tag) | $$(UpdateBook(event, event->data<OrderInput>()));
  events_ | is(Order::tag) | $$(UpdateBook(event, event->data<Order>()));
  events_ | is(Trade::tag) | $$(UpdateBook(event, event->data<Trade>()));
  events_ | is(Position::tag) | $$(UpdateBook(event, event->data<Position>()););
  events_ | is(PositionEnd::tag) | $$(UpdateAsset(event, event->data<PositionEnd>().holder_uid););
  events_ | is(Channel::tag) | $$(InspectChannel(event->gen_time(), event->data<Channel>()));
  events_ | is(Register::tag) | $$(OnRegister(event->gen_time(), event->data<Register>()));
  events_ | is(Deregister::tag) | $$(OnDeregister(event->gen_time(), event->data<Deregister>()));
  events_ | is(BrokerStateUpdate::tag) | $$(UpdateBrokerState(event->source(), event->data<BrokerStateUpdate>()));
  events_ | is(CacheReset::tag) | $$(UpdateEventCache(event));

  // events_ | is(CacheReset::tag) | $$(reset_cache(event));
}

void Watcher::Feed(const event_ptr &event) {
  SPDLOG_INFO("Watcher::Feed tid {} pid {}", std::this_thread::get_id(), getpid());
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (DataType::tag == event->msg_type()) {
      if (Quote::tag == event->msg_type()) {
  SPDLOG_INFO("Watcher::Feed 1 tid {} pid {}", std::this_thread::get_id(), getpid());
        auto quote = event->data<Quote>();
        auto uid = quote.uid();
        if (subscribed_instruments_.find(uid) != subscribed_instruments_.end()) {
          bookkeeper_.update_book(quote);
          UpdateBook(event->gen_time(), event->source(), event->dest(), quote);
          feed_state_data(event, data_bank_);
        }
      } else {
  SPDLOG_INFO("Watcher::Feed 2 tid {} pid {} type {}", std::this_thread::get_id(), getpid(), typeid(DataType).name());
        feed_state_data(event, data_bank_);
      }
    }
  }
  );
}

void Watcher::RestoreState(const location_ptr &state_location, int64_t from, int64_t to, bool sync_schema) {
  add_location(0, state_location);
  serialize::JsRestoreState(ledger_ref_, state_location)(from, to, sync_schema);
}

Napi::Value Watcher::CreateTask(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("Watcher::Watcher CreateTask tid {} pid {} this in main{}", std::this_thread::get_id(), GETPID(), uint64_t(this)); 
  greq.data = (void *)this;
  loop = uv_default_loop();
  loop_main = uv_default_loop();
  
      
  uv_queue_work(
      loop, &greq,
      [](uv_work_t *req) {
        SPDLOG_INFO("uv_thread_create::Step 1 tid {} pid {}", std::this_thread::get_id(), GETPID());
        // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        Watcher *watcher = (Watcher *)(req->data);
        SPDLOG_INFO("Watcher::Step 1 tid {} pid {} this in thread {}", std::this_thread::get_id(), GETPID(), uint64_t(watcher));
        while (watcher->start_) {
          SPDLOG_INFO("uv_thread_create::Step 24 tid {} pid {}", std::this_thread::get_id(), GETPID());
          if (!watcher->is_live() && !watcher->is_started() && watcher->is_usable()) {
            SPDLOG_INFO("uv_thread_create::Step 2 tid {} pid {}", std::this_thread::get_id(), GETPID());
            uv_mutex_lock(&watcher->mutex);
            watcher->setup();
            uv_mutex_unlock(&watcher->mutex);
          }
          if (watcher->is_live()) {
            SPDLOG_INFO("uv_thread_create::Step 3 tid {} pid {}", std::this_thread::get_id(), GETPID());
            uv_mutex_lock(&watcher->mutex);
            watcher->step();
            uv_mutex_unlock(&watcher->mutex);
            SPDLOG_INFO("uv_thread_create::Step 4 tid {} pid {}", std::this_thread::get_id(), GETPID());
          }
          if(!watcher->start_) break;
          std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
      },
      [](uv_work_t *req, int status) { SPDLOG_INFO("uv_close!!!!!"); });

      timer_req.data = (void *)this;
  uv_timer_init(loop_main, &timer_req);
  uv_timer_start(
      &timer_req,
      [](uv_timer_t *req) {
        SPDLOG_INFO("uv_timer_start 1 tid {} pid {}", std::this_thread::get_id(), GETPID());
        Watcher *watcher = (Watcher *)(req->data);
        SPDLOG_INFO("uv_timer_start 2 tid {} pid {} this {}", std::this_thread::get_id(), GETPID(), (uint64_t)(watcher));
        uv_mutex_lock(&watcher->mutex);
        watcher->SyncLedger();
        watcher->SyncAppStatus();
        watcher->SyncEventCache();
        uv_mutex_unlock(&watcher->mutex);
      },
      0, 2000);
  
  return {};
}

void Watcher::SyncLedger() {
  SPDLOG_INFO("Watcher::SyncLedger 1 tid {} pid {}", std::this_thread::get_id(), GETPID());
  boost::hana::for_each(longfist::StateDataTypes, [&](auto it) {
    UpdateLedger(+boost::hana::second(it));
  });
  SPDLOG_INFO("Watcher::SyncLedger 2 tid {} pid {}", std::this_thread::get_id(), GETPID());
}

void Watcher::SyncAppStatus() {
  SPDLOG_INFO("SyncAppStatus 1 tid {} pid {}", std::this_thread::get_id(), GETPID());
  for(auto &s : location_uid_states_map_ ){
    auto app_state = Napi::Number::New(app_states_ref_.Env(), s.second);
    app_states_ref_.Set(format(s.first), app_state);
  }
  SPDLOG_INFO("SyncAppStatus 2 tid {} pid {}", std::this_thread::get_id(), GETPID());
}

void Watcher::SyncEventCache() {
  SPDLOG_INFO("SyncEventCache 1 tid {} pid {}", std::this_thread::get_id(), GETPID());
  if(event_cache_){
    reset_cache(event_cache_);
  }
  SPDLOG_INFO("SyncEventCache 2 tid {} pid {}", std::this_thread::get_id(), GETPID());
}

void Watcher::UpdateEventCache(const event_ptr e) {
  event_cache_ = e;
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
            location_uid_states_map_.insert_or_assign(md_location->uid, int(BrokerState::Ready));
            // auto ready = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Ready));
            // app_states_ref_.Set(format(md_location->uid), ready);
            events_ | from(md_location->uid) | is(Quote::tag) | timeout(std::chrono::seconds(5)) |
                $(noop_event_handler(), [&, trigger_time, md_location](std::exception_ptr e) {
                  if (is_location_live(md_location->uid)) {
                    location_uid_states_map_.insert_or_assign(md_location->uid, int(BrokerState::Idle));
                    // auto idle = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Idle));
                    // app_states_ref_.Set(format(md_location->uid), idle);
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
    location_uid_states_map_.insert_or_assign(app_location->uid, int(BrokerState::Connected));
    // auto state = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Connected));
    // app_states_ref_.Set(format(app_location->uid), state);
  }

  if (app_location->category == category::MD and app_location->mode == mode::LIVE) {
    MonitorMarketData(trigger_time, app_location);
  }
}

void Watcher::OnDeregister(int64_t trigger_time, const Deregister &deregister_data) {
  auto app_location = location::make_shared(deregister_data, get_locator());
  location_uid_states_map_.insert_or_assign(app_location->uid, int(BrokerState::Pending));
  // auto state = Napi::Number::New(app_states_ref_.Env(), int(BrokerState::Pending));
  // app_states_ref_.Set(format(app_location->uid), state);
}

void Watcher::UpdateBrokerState(uint32_t broker_uid, const BrokerStateUpdate &state) {
  auto app_location = get_location(broker_uid);
  location_uid_states_map_.insert_or_assign(app_location->uid, int(state.state));
  // auto state_value = Napi::Number::New(app_states_ref_.Env(), int(state.state));
  // app_states_ref_.Set(format(app_location->uid), state_value);
}

void Watcher::UpdateAsset(const event_ptr &event, uint32_t book_uid) {
  auto book = bookkeeper_.get_book(book_uid);
  book->update(event->gen_time());
  // update_ledger(event->gen_time(), ledger_location_->uid, book_uid, book->asset);
  state<kungfu::longfist::types::Asset> cache_state(ledger_location_->uid, book_uid, event->gen_time() , book->asset);
  feed_state_data_bank(cache_state, data_bank_);
}

void Watcher::UpdateBook(const event_ptr &event, const Quote &quote) {
  auto ledger_uid = ledger_location_->uid;
  for (const auto &item : bookkeeper_.get_books()) {
    auto &book = item.second;
    auto holder_uid = book->asset.holder_uid;

    if (holder_uid == ledger_uid) {
      continue;
    }

    bool has_long_position_for_quote = book->has_long_position_for(quote);
    bool has_short_position_for_quote = book->has_short_position_for(quote);

    if (has_long_position_for_quote) {
      UpdateBook(event, book->get_position_for(Direction::Long, quote));
    }
    if (has_short_position_for_quote) {
      UpdateBook(event, book->get_position_for(Direction::Short, quote));
    }

    if (has_short_position_for_quote or has_long_position_for_quote) {
      // update_ledger(event->gen_time(), ledger_uid, holder_uid, book->asset);
      state<kungfu::longfist::types::Asset> cache_state(ledger_uid, holder_uid, event->gen_time() , book->asset);
      feed_state_data_bank(cache_state, data_bank_);
    }
  }
}

void Watcher::UpdateBook(int64_t update_time, uint32_t source_id, uint32_t dest_id,
                         const longfist::types::Quote &quote) {
  auto ledger_uid = ledger_location_->uid;
  for (const auto &item : bookkeeper_.get_books()) {
    auto &book = item.second;
    auto holder_uid = book->asset.holder_uid;

    if (holder_uid == ledger_uid) {
      continue;
    }

    bool has_long_position_for_quote = book->has_long_position_for(quote);
    bool has_short_position_for_quote = book->has_short_position_for(quote);

    if (has_long_position_for_quote) {
      UpdateBook(update_time, source_id, dest_id, book->get_position_for(Direction::Long, quote));
    }
    if (has_short_position_for_quote) {
      UpdateBook(update_time, source_id, dest_id, book->get_position_for(Direction::Short, quote));
    }

    if (has_short_position_for_quote or has_long_position_for_quote) {
      state<kungfu::longfist::types::Asset> cache_state(ledger_uid, holder_uid, update_time , book->asset);
      feed_state_data_bank(cache_state, data_bank_);
      // update_ledger(update_time, ledger_uid, holder_uid, book->asset);
    }
  }
}

void Watcher::UpdateBook(const event_ptr &event, const Position &position) {
  auto book = bookkeeper_.get_book(position.holder_uid);
  auto &book_position = book->get_position_for(position.direction, position);
  if (book_position.volume > 0 or book_position.direction == Direction::Long) {
    state<kungfu::longfist::types::Position> cache_state(event->source(), event->dest(), event->gen_time(), book_position);
    feed_state_data_bank(cache_state, data_bank_);
    // update_ledger(event, book_position);
  }
}

void Watcher::UpdateBook(int64_t update_time, uint32_t source_id, uint32_t dest_id,
                         const longfist::types::Position &position) {
  auto book = bookkeeper_.get_book(position.holder_uid);
  auto &book_position = book->get_position_for(position.direction, position);
  if (book_position.volume > 0 or book_position.direction == Direction::Long) {
      state<kungfu::longfist::types::Position> cache_state(source_id, dest_id, update_time, book_position);
      feed_state_data_bank(cache_state, data_bank_);
    // update_ledger(update_time, source_id, dest_id, book_position);
  }
}

Napi::Value Watcher::Lock(const Napi::CallbackInfo &info) { uv_mutex_lock(&mutex); return {}; }
Napi::Value Watcher::Unlock(const Napi::CallbackInfo &info) { uv_mutex_unlock(&mutex); return {}; }

} // namespace kungfu::node