//
// Created by Keren Dong on 2020/3/12.
//

#ifndef WINGCHUN_BROKER_CLIENT_H
#define WINGCHUN_BROKER_CLIENT_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/broker.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker {
class Client {
public:
  explicit Client(yijinjing::practice::apprentice &app, bool monitor_all_md = false, bool monitor_all_td = false);

  [[nodiscard]] const std::unordered_map<uint32_t, longfist::types::Instrument> &get_instruments() const;

  void subscribe(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id,
                 const std::string &instrument_id);

  void subscribe_all(const yijinjing::data::location_ptr &md_location);

  [[nodiscard]] bool is_subscribed(uint32_t md_location_uid, const std::string &exchange_id,
                                   const std::string &instrument_id) const;

  [[nodiscard]] bool is_subscribed(const yijinjing::data::location_ptr &md_location, const std::string &exchange_id,
                                   const std::string &instrument_id) const;

  [[nodiscard]] bool is_all_subscribed(const yijinjing::data::location_ptr &md_location) const;

  void enroll_account(const yijinjing::data::location_ptr &td_location);

  [[nodiscard]] bool is_ready(uint32_t broker_location_uid) const;

  void on_start(const rx::connectable_observable<event_ptr> &events);

private:
  yijinjing::practice::apprentice &app_;
  const bool monitor_all_md_;
  const bool monitor_all_td_;

  std::unordered_map<uint32_t, longfist::enums::BrokerState> broker_states_;

  std::unordered_map<uint32_t, bool> enrolled_md_locations_;
  std::unordered_map<uint32_t, bool> enrolled_td_locations_;

  std::unordered_map<uint32_t, yijinjing::data::location_ptr> ready_md_locations_;
  std::unordered_map<uint32_t, yijinjing::data::location_ptr> ready_td_locations_;

  std::unordered_map<uint32_t, yijinjing::data::location_ptr> instrument_md_locations_;
  std::unordered_map<uint32_t, longfist::types::Instrument> instruments_;

  [[nodiscard]] uint32_t hash_instrument(const longfist::types::Instrument &instrument) const;

  [[nodiscard]] bool is_enrolled_md(const yijinjing::data::location_ptr &md_location) const;

  [[nodiscard]] bool is_enrolled_td(const yijinjing::data::location_ptr &md_location) const;

  void connect(const longfist::types::Register &register_data);
};
} // namespace kungfu::wingchun::broker

#endif // WINGCHUN_BROKER_CLIENT_H
