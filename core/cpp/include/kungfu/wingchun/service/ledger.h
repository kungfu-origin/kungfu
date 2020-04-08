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

  std::unordered_map<uint64_t, state<longfist::types::Asset>> assets_ = {};
  std::unordered_map<uint64_t, state<longfist::types::OrderStat>> order_stats_ = {};

  longfist::types::OrderStat &get_order_stat(uint64_t order_id, const event_ptr &event);

  void write_book_reset(int64_t trigger_time, uint32_t dest);

  void write_strategy_data(int64_t trigger_time, uint32_t dest);

  void write_daily_assets();

  template <typename DataType> void write_book(const event_ptr &event, const DataType &data) {
    auto source = get_location(event->source());
    write_book(event->gen_time(), event->source(), data, source->group, source->name, "");
    if (event->dest()) {
      auto dest = get_location(event->dest());
      write_book(event->gen_time(), event->dest(), data, source->group, source->name, dest->name);
    }
  }

  void write_book(const event_ptr &event, const longfist::types::OrderInput &data) {
    auto source = get_location(event->source());
    auto dest = get_location(event->dest());
    write_book(event->gen_time(), event->source(), data, dest->group, dest->name, source->name);
    write_book(event->gen_time(), event->dest(), data, dest->group, dest->name, "");
  }

  template <typename DataType>
  void write_book(int64_t trigger_time, const uint32_t location_uid, const DataType &data, const std::string &source_id,
                  const std::string &account_id, const std::string &client_id) {
    auto book = bookkeeper_.get_book(location_uid);
    auto &position = book->get_position(data);
    strcpy(position.source_id, source_id.c_str());
    strcpy(position.account_id, account_id.c_str());
    strcpy(position.client_id, client_id.c_str());
    write_to(trigger_time, position, location_uid);
    write_to(trigger_time, book->asset, location_uid);
  }
};
} // namespace kungfu::wingchun::service

#endif // WINGCHUN_LEDGER_H
