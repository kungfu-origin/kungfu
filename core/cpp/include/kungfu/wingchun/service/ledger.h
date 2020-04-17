//
// Created by Keren Dong on 2019-06-28.
//

#ifndef WINGCHUN_LEDGER_H
#define WINGCHUN_LEDGER_H

#include <kungfu/wingchun/book/bookkeeper.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::service {
class Ledger : public yijinjing::practice::apprentice {
public:
  explicit Ledger(yijinjing::data::locator_ptr locator, longfist::enums::mode m, bool low_latency = false);

  ~Ledger() override = default;

  void on_exit() override;

  void on_trading_day(const event_ptr &event, int64_t daytime) override;

  book::Bookkeeper &get_bookkeeper();

protected:
  void on_start() override;

private:
  broker::AutoClient broker_client_;
  book::Bookkeeper bookkeeper_;
  std::unordered_map<uint64_t, state<longfist::types::OrderStat>> order_stats_ = {};

  void refresh_books();

  void refresh_account_book(int64_t trigger_time, uint32_t account_uid);

  longfist::types::OrderStat &get_order_stat(uint64_t order_id, const event_ptr &event);

  void update_order_stat(const event_ptr &event, const longfist::types::OrderInput &data);

  void update_order_stat(const event_ptr &event, const longfist::types::Order &data);

  void update_order_stat(const event_ptr &event, const longfist::types::Trade &data);

  void update_account_book(int64_t trigger_time, uint32_t account_uid);

  void inspect_channel(int64_t trigger_time, const longfist::types::Channel &channel);

  void mirror_positions(int64_t trigger_time, uint32_t strategy_uid);

  void write_book_reset(int64_t trigger_time, uint32_t book_uid);

  void write_strategy_data(int64_t trigger_time, uint32_t strategy_uid);

  void write_asset_snapshots(int32_t msg_type);

  template <typename TradingData>
  void write_book(int64_t trigger_time, uint32_t account_uid, uint32_t strategy_uid, const TradingData &data) {
    write_book(trigger_time, account_uid, data);
    write_book(trigger_time, strategy_uid, data);
  }

  template <typename TradingData> void write_book(int64_t trigger_time, uint32_t book_uid, const TradingData &data) {
    if (not bookkeeper_.has_book(book_uid) or not has_writer(book_uid)) {
      return;
    }
    auto book = bookkeeper_.get_book(book_uid);
    write_to(trigger_time, book->get_position(data), book_uid);
    write_to(trigger_time, book->asset, book_uid);
  }

  template <typename Writer, typename Snapshot>
  static void write_asset_snapshot(int64_t trigger_time, Writer &&writer, const Snapshot &snapshot) {
    writer->write(trigger_time, longfist::types::AssetSnapshot::tag, snapshot);
    writer->write(trigger_time, longfist::types::DailyAsset::tag, snapshot);
  }
};
} // namespace kungfu::wingchun::service

#endif // WINGCHUN_LEDGER_H
