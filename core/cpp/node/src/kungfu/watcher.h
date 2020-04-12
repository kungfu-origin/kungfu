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
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::node {
constexpr uint64_t ID_TRANC = 0x00000000FFFFFFFF;

class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice {
public:
  explicit Watcher(const Napi::CallbackInfo &info);

  ~Watcher() override;

  void NoSet(const Napi::CallbackInfo &info, const Napi::Value &value);

  Napi::Value GetLocator(const Napi::CallbackInfo &info);

  Napi::Value GetConfig(const Napi::CallbackInfo &info);

  Napi::Value GetHistory(const Napi::CallbackInfo &info);

  Napi::Value GetCommission(const Napi::CallbackInfo &info);

  Napi::Value GetState(const Napi::CallbackInfo &info);

  Napi::Value GetLedger(const Napi::CallbackInfo &info);

  Napi::Value GetAppStates(const Napi::CallbackInfo &info);

  Napi::Value GetTradingDay(const Napi::CallbackInfo &info);

  Napi::Value IsUsable(const Napi::CallbackInfo &info);

  Napi::Value IsLive(const Napi::CallbackInfo &info);

  Napi::Value IsStarted(const Napi::CallbackInfo &info);

  Napi::Value Setup(const Napi::CallbackInfo &info);

  Napi::Value Step(const Napi::CallbackInfo &info);

  Napi::Value GetLocation(const Napi::CallbackInfo &info);

  Napi::Value PublishState(const Napi::CallbackInfo &info);

  Napi::Value IsReadyToInteract(const Napi::CallbackInfo &info);

  Napi::Value IssueOrder(const Napi::CallbackInfo &info);

  Napi::Value CancelOrder(const Napi::CallbackInfo &info);

  static void Init(Napi::Env env, Napi::Object exports);

protected:
  void on_react() override;

  void on_start() override;

private:
  static Napi::FunctionReference constructor;
  yijinjing::data::location_ptr ledger_location_;
  wingchun::broker::AutoClient broker_client_;
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
  std::unordered_map<uint32_t, yijinjing::data::location_ptr> proxy_locations_;

  void RestoreState(const yijinjing::data::location_ptr &config_location, int64_t from, int64_t to);

  yijinjing::data::location_ptr FindLocation(const Napi::CallbackInfo &info);

  void MonitorMarketData(int64_t trigger_time, const yijinjing::data::location_ptr &md_location);

  void UpdateBook(const event_ptr &event, const longfist::types::Quote &quote) {
    using namespace kungfu::longfist::enums;
    auto ledger_uid = ledger_location_->uid;
    for (const auto &item : bookkeeper_.get_books()) {
      auto &book = item.second;
      auto holder_uid = book->asset.holder_uid;
      auto not_ledger = holder_uid != ledger_uid;
      auto has_long_position = book->has_long_position(quote) and not_ledger;
      auto has_short_position = book->has_short_position(quote) and not_ledger;
      if (has_long_position) {
        update_ledger(event->gen_time(), ledger_uid, holder_uid, book->get_position(Direction::Long, quote));
      }
      if (has_short_position) {
        update_ledger(event->gen_time(), ledger_uid, holder_uid, book->get_position(Direction::Short, quote));
      }
      if (has_long_position or has_short_position) {
        update_ledger(event->gen_time(), ledger_uid, holder_uid, book->asset);
      }
    }
  }

  template <typename DataType> void UpdateBook(const event_ptr &event, const DataType &data) {
    auto update = [&](uint32_t source, uint32_t dest) {
      auto location = get_location(source);
      auto book = bookkeeper_.get_book(source);
      auto &position = book->get_position(data);
      update_ledger(event->gen_time(), source, dest, position);
      update_ledger(event->gen_time(), source, dest, book->asset);
    };
    update(event->source(), event->dest());
    update(event->dest(), event->source());
  }

  template <typename DataType, typename IdPtrType = uint64_t DataType::*>
  Napi::Value InteractWithTD(const Napi::CallbackInfo &info, IdPtrType id_ptr) {
    try {
      using namespace kungfu::rx;
      using namespace kungfu::longfist::types;
      using namespace kungfu::yijinjing;
      using namespace kungfu::yijinjing::data;

      auto trigger_time = time::now_in_nano();
      Napi::Object obj = info[0].ToObject();
      DataType action = {};
      serialize::JsGet{}(obj, action);
      auto account_location = ExtractLocation(info, 1, get_locator());
      if (not account_location or not has_writer(account_location->uid)) {
        return Napi::Boolean::New(info.Env(), false);
      }

      auto account_writer = get_writer(account_location->uid);
      auto master_cmd_writer = get_writer(get_master_commands_uid());

      if (info.Length() == 2) {
        action.*id_ptr = account_writer->current_frame_uid();
        account_writer->write(trigger_time, action);
        return Napi::Boolean::New(info.Env(), true);
      }

      auto strategy_location = ExtractLocation(info, 2, get_locator());
      if (not strategy_location or not has_location(strategy_location->uid)) {
        return Napi::Boolean::New(info.Env(), false);
      }

      auto proxy_location = location::make_shared(strategy_location->mode, strategy_location->category,
                                                  get_io_device()->get_home()->group, strategy_location->name,
                                                  strategy_location->locator);

      auto ensure_location = [&](auto location) {
        if (not has_location(location->uid)) {
          add_location(trigger_time, location);
          master_cmd_writer->write(trigger_time, *std::dynamic_pointer_cast<Location>(location));
        }
      };
      auto perform = [trigger_time, account_location, proxy_location, account_writer, action, id_ptr]() {
        uint64_t id_left = (uint64_t)(proxy_location->uid xor account_location->uid) << 32u;
        uint64_t id_right = ID_TRANC & account_writer->current_frame_uid();
        DataType data = action;
        data.*id_ptr = id_left | id_right;
        account_writer->write_as(trigger_time, data, proxy_location->uid);
        SPDLOG_INFO("{} sent to {} order {}", proxy_location->uname, account_location->uname, data.to_string());
      };

      ensure_location(strategy_location);
      ensure_location(proxy_location);

      if (has_channel(account_location->uid, proxy_location->uid)) {
        perform();
        return Napi::Boolean::New(info.Env(), true);
      }

      events_ | is(Channel::tag) | filter([account_location, proxy_location](const event_ptr &event) {
        const Channel &channel = event->data<Channel>();
        return channel.source_id == account_location->uid and channel.dest_id == proxy_location->uid;
      }) | first() |
          $([perform](const event_ptr &event) { perform(); },
            error_handler_log(fmt::format("channel {} -> {}", account_location->uname, proxy_location->uname)));
      Channel request = {};
      request.dest_id = proxy_location->uid;
      request.source_id = ledger_location_->uid;
      master_cmd_writer->write(trigger_time, request);
      request.source_id = account_location->uid;
      master_cmd_writer->write(trigger_time, request);
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
