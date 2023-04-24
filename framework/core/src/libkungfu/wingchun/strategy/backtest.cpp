// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/7/20.
//

#include <kungfu/wingchun/strategy/backtest.h>

namespace kungfu::wingchun::strategy {
int64_t BacktestContext::now() const { return 0; }
void BacktestContext::add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback) {}
void BacktestContext::add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback) {}
void BacktestContext::add_account(const std::string &source, const std::string &account) {}
void BacktestContext::subscribe(const std::string &source, const std::vector<std::string> &instrument_ids,
                                const std::string &exchange_ids) {}
void BacktestContext::subscribe_all(const std::string &source, uint8_t market_type, uint64_t instrument_type,
                                    uint64_t data_type) {}
uint64_t BacktestContext::insert_order(const std::string &instrument_id, const std::string &exchange_id,
                                       const std::string &source, const std::string &account, double limit_price,
                                       int64_t volume, longfist::enums::PriceType type, longfist::enums::Side side,
                                       longfist::enums::Offset offset, longfist::enums::HedgeFlag hedge_flag,
                                       bool is_swap, uint64_t block_id, uint64_t parent_id) {
  return 0;
}
uint64_t BacktestContext::cancel_order(uint64_t order_id) { return 0; }
int64_t BacktestContext::get_trading_day() const { return 0; }
} // namespace kungfu::wingchun::strategy
