//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>

#include "common.h"
#include "io.h"
#include "journal.h"
#include "operators.h"
#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::node {
constexpr uint64_t ID_TRANC = 0x00000000FFFFFFFF;
constexpr uint32_t PAGE_ID_MASK = 0x80000000;

class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice {
public:
  explicit Watcher(const Napi::CallbackInfo &info);

  ~Watcher() override;

  void NoSet(const Napi::CallbackInfo &info, const Napi::Value &value);

  Napi::Value GetLocator(const Napi::CallbackInfo &info);

  Napi::Value GetLocation(const Napi::CallbackInfo &info);

  Napi::Value GetLocationUID(const Napi::CallbackInfo &info);

  Napi::Value GetInstrumentUID(const Napi::CallbackInfo &info);

  Napi::Value GetConfig(const Napi::CallbackInfo &info);

  Napi::Value GetHistory(const Napi::CallbackInfo &info);

  Napi::Value GetCommission(const Napi::CallbackInfo &info);

  Napi::Value GetState(const Napi::CallbackInfo &info);

  Napi::Value GetLedger(const Napi::CallbackInfo &info);

  Napi::Value GetAppStates(const Napi::CallbackInfo &info);

  Napi::Value GetTradingDay(const Napi::CallbackInfo &info);

  Napi::Value Now(const Napi::CallbackInfo &info);

  Napi::Value IsUsable(const Napi::CallbackInfo &info);

  Napi::Value IsLive(const Napi::CallbackInfo &info);

  Napi::Value IsStarted(const Napi::CallbackInfo &info);

  Napi::Value Setup(const Napi::CallbackInfo &info);

  Napi::Value Step(const Napi::CallbackInfo &info);

  Napi::Value RequestStop(const Napi::CallbackInfo &info);

  Napi::Value PublishState(const Napi::CallbackInfo &info);

  Napi::Value IsReadyToInteract(const Napi::CallbackInfo &info);

  Napi::Value IssueOrder(const Napi::CallbackInfo &info);

  Napi::Value CancelOrder(const Napi::CallbackInfo &info);

  Napi::Value RequestMarketData(const Napi::CallbackInfo &info);

  void UpdateQuote(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

protected:
  void on_react() override;

  void on_start() override;

private:
  static Napi::FunctionReference constructor;
  const bool bypass_quotes_;
  yijinjing::data::location_ptr ledger_location_;
  wingchun::broker::SilentAutoClient broker_client_;
  wingchun::book::Bookkeeper bookkeeper_;
  Napi::ObjectReference history_ref_;
  Napi::ObjectReference config_ref_;
  Napi::ObjectReference commission_ref_;
  Napi::ObjectReference state_ref_;
  Napi::ObjectReference ledger_ref_;
  Napi::ObjectReference app_states_ref_;
  serialize::JsUpdateState update_state;
  serialize::JsUpdateState update_ledger;
  serialize::JsPublishState publish;
  serialize::JsResetCache reset_cache;
  yijinjing::cache::bank quotes_bank_;
  std::unordered_map<uint32_t, longfist::types::InstrumentKey> subscribed_instruments_ = {};

  static constexpr auto bypass = [](yijinjing::practice::apprentice *app, bool bypass_quotes) {
    return rx::filter([=](const event_ptr &event) {
      return not(app->get_location(event->source())->category == longfist::enums::category::MD and bypass_quotes);
    });
  };

  void Feed(const event_ptr &event);

  void RestoreState(const yijinjing::data::location_ptr &state_location, int64_t from, int64_t to, bool sync_schema);

  yijinjing::data::location_ptr FindLocation(const Napi::CallbackInfo &info);

  void InspectChannel(int64_t trigger_time, const longfist::types::Channel &channel);

  void MonitorMarketData(int64_t trigger_time, const yijinjing::data::location_ptr &md_location);

  void OnRegister(int64_t trigger_time, const longfist::types::Register &register_data);

  void OnDeregister(int64_t trigger_time, const longfist::types::Deregister &deregister_data);

  void UpdateBrokerState(uint32_t broker_uid, const longfist::types::BrokerStateUpdate &state);

  void UpdateAsset(const event_ptr &event, uint32_t book_uid);

  void UpdateBook(const event_ptr &event, const longfist::types::Quote &quote);

  void UpdateBook(int64_t update_time, uint32_t source_id, uint32_t dest_id, const longfist::types::Quote &quote);

  void UpdateBook(const event_ptr &event, const longfist::types::Position &position);

  void UpdateBook(int64_t update_time, uint32_t source_id, uint32_t dest_id, const longfist::types::Position &position);

  template <typename TradingData> void UpdateBook(const event_ptr &event, const TradingData &data) {
    auto update = [&](uint32_t source, uint32_t dest) {
      if (source == yijinjing::data::location::PUBLIC) {
        return;
      }
      auto location = get_location(source);
      auto book = bookkeeper_.get_book(source);
      auto &position = book->get_position_for(data);
      update_ledger(event->gen_time(), source, dest, position);
      update_ledger(event->gen_time(), source, dest, book->asset);
    };
    update(event->source(), event->dest());
    update(event->dest(), event->source());
  }

  template <typename TradingData>
  std::enable_if_t<std::is_same_v<TradingData, longfist::types::OrderInput>> UpdateBook(uint32_t source, uint32_t dest,
                                                                                        const TradingData &data) {
    bookkeeper_.on_order_input(now(), source, dest, data);
  }

  template <typename TradingData>
  std::enable_if_t<not std::is_same_v<TradingData, longfist::types::OrderInput>>
  UpdateBook(uint32_t source, uint32_t dest, const TradingData &data) {}

  template <typename Instruction, typename IdPtrType = uint64_t Instruction::*>
  void WriteInstruction(int64_t trigger_time, Instruction instruction, IdPtrType id_ptr,
                        const yijinjing::data::location_ptr &account_location,
                        const yijinjing::data::location_ptr &strategy_location) {
    auto account_writer = get_writer(account_location->uid);
    uint64_t id_left = (uint64_t)(strategy_location->uid xor account_location->uid) << 32u;
    uint64_t id_right = (ID_TRANC & account_writer->current_frame_uid()) | PAGE_ID_MASK;
    instruction.*id_ptr = id_left | id_right;
    account_writer->write_as(trigger_time, instruction, strategy_location->uid, account_location->uid);
    UpdateBook(strategy_location->uid, account_location->uid, instruction);
  }

  template <typename Instruction, typename IdPtrType = uint64_t Instruction::*>
  Napi::Value InteractWithTD(const Napi::CallbackInfo &info, IdPtrType id_ptr) {
    try {
      using namespace kungfu::rx;
      using namespace kungfu::longfist::types;
      using namespace kungfu::yijinjing;
      using namespace kungfu::yijinjing::data;

      auto account_location = ExtractLocation(info, 1, get_locator());
      if (not is_location_live(account_location->uid) or not has_writer(account_location->uid)) {
        return Napi::Boolean::New(info.Env(), false);
      }

      auto trigger_time = time::now_in_nano();
      auto account_writer = get_writer(account_location->uid);
      auto master_cmd_writer = get_writer(get_master_commands_uid());
      auto instruction_object = info[0].ToObject();
      Instruction instruction = {};
      serialize::JsGet{}(instruction_object, instruction);

      if (info.Length() == 2) {
        instruction.*id_ptr = account_writer->current_frame_uid();
        account_writer->write(trigger_time, instruction);
        UpdateBook(get_home_uid(), account_location->uid, instruction);
        return Napi::Boolean::New(info.Env(), true);
      }

      auto strategy_location = ExtractLocation(info, 2, get_locator());

      if (not strategy_location or not has_location(strategy_location->uid)) {
        return Napi::Boolean::New(info.Env(), false);
      }

      if (not has_location(strategy_location->uid)) {
        add_location(trigger_time, strategy_location);
        master_cmd_writer->write(trigger_time, *std::dynamic_pointer_cast<Location>(strategy_location));
      }

      if (has_channel(account_location->uid, strategy_location->uid)) {
        WriteInstruction(trigger_time, instruction, id_ptr, account_location, strategy_location);
        return Napi::Boolean::New(info.Env(), true);
      }

      Channel request = {};
      request.dest_id = strategy_location->uid;
      request.source_id = ledger_location_->uid;
      master_cmd_writer->write(trigger_time, request);
      request.source_id = account_location->uid;
      master_cmd_writer->write(trigger_time, request);

      events_ | is(Channel::tag) | filter([account_location, strategy_location](const event_ptr &event) {
        const Channel &channel = event->data<Channel>();
        return channel.source_id == account_location->uid and channel.dest_id == strategy_location->uid;
      }) | first() |
          $([this, trigger_time, instruction, id_ptr, account_location, strategy_location](auto event) {
            WriteInstruction(trigger_time, instruction, id_ptr, account_location, strategy_location);
          });

      return Napi::Boolean::New(info.Env(), true);
    } catch (const std::exception &ex) {
      throw Napi::Error::New(info.Env(), fmt::format("invalid order arguments: {}", ex.what()));
    } catch (...) {
      throw Napi::Error::New(info.Env(), "invalid order arguments");
    }
  };
};
} // namespace kungfu::node

#endif // KUNGFU_NODE_WATCHER_H
