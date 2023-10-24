// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/4/6.
//

#include <kungfu/wingchun/book/bookkeeper.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun::broker;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::book {
Bookkeeper::Bookkeeper(apprentice &app, broker::Client &broker_client, bool bypass_quote)
    : app_(app), broker_client_(broker_client), bypass_quote_(bypass_quote) {
  book::AccountingMethod::setup_defaults(*this);
  char *skip_sync_asset = std::getenv("KF_SKIP_SYNC_ASSET");
  char *skip_sync_asset_margin = std::getenv("KF_SKIP_SYNC_ASSET_MARGIN");
  char *skip_sync_position = std::getenv("KF_SKIP_SYNC_POSITION");
  sync_asset_ = skip_sync_asset == nullptr;
  sync_asset_margin_ = skip_sync_asset_margin == nullptr;
  sync_position_ = skip_sync_position == nullptr;
  SPDLOG_DEBUG("sync_asset_: {}, sync_asset_margin_: {}, sync_position_: {}", sync_asset_, sync_asset_margin_,
               sync_position_);
}

bool Bookkeeper::has_book(uint32_t location_uid) { return books_.find(location_uid) != books_.end(); }

void Bookkeeper::drop_book(uint32_t uid) { books_.erase(uid); }

Book_ptr Bookkeeper::get_book(uint32_t location_uid) {
  if (books_.find(location_uid) == books_.end()) {
    books_.emplace(location_uid, make_book(location_uid));
  }
  return books_.at(location_uid);
}

const BookMap &Bookkeeper::get_books() const { return books_; }

void Bookkeeper::set_accounting_method(InstrumentType instrument_type, const AccountingMethod_ptr &accounting_method) {
  accounting_methods_.emplace(instrument_type, accounting_method);
}

void Bookkeeper::on_trading_day(int64_t daytime) {
  auto trading_day = time::strftime(daytime, KUNGFU_TRADING_DAY_FORMAT);
  for (const auto &book_pair : books_) {
    const auto &book = book_pair.second;
    strcpy(book->asset.trading_day, trading_day.c_str());
    for (auto &pos_pair : book->long_positions) {
      pos_pair.second.trading_day = book->asset.trading_day;
    }
    for (auto &pos_pair : book->short_positions) {
      pos_pair.second.trading_day = book->asset.trading_day;
    }
  }
}

void Bookkeeper::on_start(const rx::connectable_observable<event_ptr> &events) {
  restore(app_.get_state_bank());
  on_trading_day(app_.get_trading_day());

  events | is(Instrument::tag) | $$(update_instrument(event->data<Instrument>()));
  events | is_own<Quote>(broker_client_) | $$(try_update_book(event, event->data<Quote>()));
  events | is(InstrumentKey::tag) | $$(update_book(event, event->data<InstrumentKey>()));
  events | is(OrderInput::tag) | $$(update_book<OrderInput>(event, &AccountingMethod::apply_order_input));
  events | is(Order::tag) | $$(update_book<Order>(event, &AccountingMethod::apply_order));
  events | is(Trade::tag) | $$(update_book<Trade>(event, &AccountingMethod::apply_trade));
  events | fork<Asset>(location::SYNC, &Bookkeeper::try_sync_asset, &Bookkeeper::try_update_asset);
  events | fork<AssetMargin>(location::SYNC, &Bookkeeper::try_sync_asset_margin, &Bookkeeper::try_update_asset_margin);
  events | fork<Position>(location::SYNC, &Bookkeeper::try_sync_position, &Bookkeeper::try_update_position);
  events | fork<PositionEnd>(location::SYNC, &Bookkeeper::try_sync_position_end, &Bookkeeper::try_update_position_end);
  events | is(TradingDay::tag) | $$(on_trading_day(event->data<TradingDay>().timestamp));
  events | is(ResetBookRequest::tag) | $$(drop_book(event->source()));

  if (bypass_quote_) {
    app_.add_time_interval(yijinjing::time_unit::NANOSECONDS_PER_SECOND * 15,
                           [&](auto e) { batch_update_book_by_quote(); });
  }
}

void Bookkeeper::batch_update_book_by_quote() {
  SPDLOG_DEBUG("batch_update_book_by_quote");

  for (const auto &iter : quotes_) {
    const auto &state_quote = iter.second;
    update_book(state_quote.update_time, state_quote.data);
  }
  quotes_.clear();
}

std::mutex &Bookkeeper::get_update_book_mutex() { return update_book_mutex_; }

void Bookkeeper::try_update_position_end(const PositionEnd &position_end) {
  get_book(position_end.holder_uid)->update(app_.now());
}

void Bookkeeper::on_order_input(int64_t update_time, uint32_t source, uint32_t dest, const OrderInput &input) {
  update_book<OrderInput>(update_time, source, dest, input, &AccountingMethod::apply_order_input);
}

void Bookkeeper::restore(const cache::bank &state_bank) {
  for (auto &pair : state_bank[boost::hana::type_c<Instrument>]) {
    update_instrument(pair.second.data);
  }
  for (auto &pair : state_bank[boost::hana::type_c<Commission>]) {
    auto &state = pair.second;
    auto &commission = state.data;
    commissions_.emplace(hash_str_32(commission.product_id), commission);
  }
  for (auto &pair : state_bank[boost::hana::type_c<Position>]) {
    auto &state = pair.second;
    auto &position = state.data;
    if (not app_.has_location(position.holder_uid)) {
      continue;
    }
    auto book = get_book(position.holder_uid);
    auto is_long = position.direction == longfist::enums::Direction::Long;
    auto &positions = is_long ? book->long_positions : book->short_positions;
    positions[hash_instrument(position.exchange_id, position.instrument_id)] = position;
  }
  for (auto &pair : state_bank[boost::hana::type_c<Asset>]) {
    auto &state = pair.second;
    auto &asset = state.data;
    if (not app_.has_location(asset.holder_uid)) {
      continue;
    }
    auto book = get_book(asset.holder_uid);
    book->asset = asset;
    book->update(app_.now());
  }
  for (auto &pair : state_bank[boost::hana::type_c<AssetMargin>]) {
    auto &state = pair.second;
    auto &asset_margin = state.data;
    if (not app_.has_location(asset_margin.holder_uid)) {
      continue;
    }
    auto book = get_book(asset_margin.holder_uid);
    book->asset_margin = asset_margin;
    book->update(app_.now());
  }
}

void Bookkeeper::guard_positions() { positions_guarded_ = true; }

Book_ptr Bookkeeper::make_book(uint32_t location_uid) {
  auto location = app_.get_location(location_uid);
  auto book = std::make_shared<Book>(commissions_, instruments_);
  auto &asset = book->asset;
  asset.holder_uid = location_uid;
  asset.ledger_category = location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
  strcpy(asset.trading_day, time::strftime(app_.get_trading_day(), KUNGFU_TRADING_DAY_FORMAT).c_str());
  auto &asset_margin = book->asset_margin;
  asset_margin.holder_uid = location_uid;
  asset_margin.ledger_category =
      location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
  strcpy(asset_margin.trading_day, time::strftime(app_.get_trading_day(), KUNGFU_TRADING_DAY_FORMAT).c_str());
  return book;
}

void Bookkeeper::update_instrument(const longfist::types::Instrument &instrument) {
  auto pair = instruments_.try_emplace(hash_instrument(instrument.exchange_id, instrument.instrument_id), instrument);
  if (not pair.second) {
    auto &inserted_inst = pair.first->second;
    if (instrument.force_update_ratio) {
      inserted_inst.long_margin_ratio = instrument.long_margin_ratio;
      inserted_inst.short_margin_ratio = instrument.short_margin_ratio;
      inserted_inst.conversion_rate = instrument.conversion_rate;
    } else {
      if (inserted_inst.force_update_ratio) {
        double long_margin_ratio = inserted_inst.long_margin_ratio;
        double short_margin_ratio = inserted_inst.short_margin_ratio;
        double conversion_rate = inserted_inst.conversion_rate;
        memcpy(&inserted_inst, &instrument, sizeof(longfist::types::Instrument));
        inserted_inst.long_margin_ratio = long_margin_ratio;
        inserted_inst.short_margin_ratio = short_margin_ratio;
        inserted_inst.conversion_rate = conversion_rate;
      } else {
        memcpy(&inserted_inst, &instrument, sizeof(longfist::types::Instrument));
      }
    }
  }
}

void Bookkeeper::update_book(const event_ptr &event, const InstrumentKey &instrument_key) {
  std::lock_guard<std::mutex> lock(update_book_mutex_);
  broker_client_.subscribe(instrument_key);
  get_book(event->source())->ensure_position(instrument_key);
}

void Bookkeeper::try_update_book(const event_ptr &event, const Quote &quote) {
  if (bypass_quote_) {
    state<Quote> state_quote(event->source(), event->dest(), event->gen_time(), quote);
    auto hashed_instrument_key = hash_instrument(quote.exchange_id, quote.instrument_id);
    quotes_.insert_or_assign(hashed_instrument_key, state_quote);
    return;
  }

  update_book(event->gen_time(), quote);
}

void Bookkeeper::update_book(int64_t trigger_time, const Quote &quote) {
  std::lock_guard<std::mutex> lock(update_book_mutex_);
  if (accounting_methods_.find(quote.instrument_type) == accounting_methods_.end()) {
    return;
  }
  auto accounting_method = accounting_methods_.at(quote.instrument_type);
  for (auto &item : books_) {
    auto &book = item.second;
    auto has_long_position = book->has_long_position_for(quote);
    auto has_short_position = book->has_short_position_for(quote);
    if (has_long_position or has_short_position) {
      accounting_method->apply_quote(book, quote);
      book->update(trigger_time);
    }
    if (has_long_position) {
      book->get_position_for(Direction::Long, quote).update_time = trigger_time;
    }
    if (has_short_position) {
      book->get_position_for(Direction::Short, quote).update_time = trigger_time;
    }
  }
}

void Bookkeeper::try_update_asset(const Asset &asset) {
  if (app_.has_location(asset.holder_uid)) {
    get_book(asset.holder_uid)->asset = asset;
  }
}

void Bookkeeper::try_update_asset_margin(const AssetMargin &asset_margin) {
  if (app_.has_location(asset_margin.holder_uid)) {
    get_book(asset_margin.holder_uid)->asset_margin = asset_margin;
  }
}

void Bookkeeper::try_update_position(const Position &position) {
  if (not app_.has_location(position.holder_uid)) {
    return;
  }
  auto book = get_book(position.holder_uid);
  auto &target_position = book->get_position_for(position.direction, position);
  if (positions_guarded_ and target_position.update_time >= position.update_time) {
    return;
  }
  auto last_price = target_position.last_price;
  target_position = position;
  target_position.last_price = std::max(last_price, target_position.last_price);
  if (accounting_methods_.find(target_position.instrument_type) == accounting_methods_.end()) {
    return;
  }
  accounting_methods_.at(target_position.instrument_type)->update_position(book, target_position);
}

void Bookkeeper::try_sync_asset(const longfist::types::Asset &asset) {
  if (not sync_asset_ or not app_.has_location(asset.holder_uid)) {
    return;
  }

  get_book_replica(asset.holder_uid)->asset = asset;
  auto old_book = get_book(asset.holder_uid);
  bool asset_changed = false;
  auto asset_compare = [](const Asset &old_asset, const Asset &new_asset) {
    bool changed = false;
    changed |= old_asset.avail != new_asset.avail;   // 可用资金
    changed |= old_asset.margin != new_asset.margin; // 保证金(期货)
    return changed;
  };
  asset_changed |= asset_compare(old_book->asset, asset);

  if (asset_changed) {
    for (auto &book_listener : book_listeners_) {
      book_listener->on_asset_sync_reset(old_book->asset, asset);
    }
  }
  longfist::copy(old_book->asset, asset);
}

void Bookkeeper::try_sync_asset_margin(const longfist::types::AssetMargin &asset_margin) {
  if (not sync_asset_margin_ or not app_.has_location(asset_margin.holder_uid)) {
    return;
  }

  get_book_replica(asset_margin.holder_uid)->asset_margin = asset_margin;
  auto old_book = get_book(asset_margin.holder_uid);
  bool asset_margin_changed = false;
  auto asset_margin_compare = [](const AssetMargin &old_asset_margin, const AssetMargin &new_asset_margin) {
    bool changed = false;
    changed |= old_asset_margin.total_asset != new_asset_margin.total_asset;   // 总资产
    changed |= old_asset_margin.avail_margin != new_asset_margin.avail_margin; // 可用保证金
    changed |= old_asset_margin.cash_margin != new_asset_margin.cash_margin;   // 融资占用保证金
    changed |= old_asset_margin.short_margin != new_asset_margin.short_margin; // 融券占用保证金
    changed |= old_asset_margin.margin != new_asset_margin.margin;             // 总占用保证金
    changed |= old_asset_margin.cash_debt != new_asset_margin.cash_debt;       // 融资负债
    changed |= old_asset_margin.short_cash != new_asset_margin.short_cash;     // 融券卖出金额
    return changed;
  };
  asset_margin_changed |= asset_margin_compare(old_book->asset_margin, asset_margin);

  if (asset_margin_changed) {
    for (auto &book_listener : book_listeners_) {
      book_listener->on_asset_margin_sync_reset(old_book->asset_margin, asset_margin);
    }
  }
  longfist::copy(old_book->asset_margin, asset_margin);
}

void Bookkeeper::try_sync_position(const longfist::types::Position &position) {
  if (not sync_position_ or not app_.has_location(position.holder_uid)) {
    return;
  }
  auto book = get_book_replica(position.holder_uid);
  auto &target_position = book->get_position_for(position.direction, position);
  target_position = position;
}

Book_ptr Bookkeeper::get_book_replica(uint32_t location_uid) {
  if (books_replica_.find(location_uid) == books_replica_.end()) {
    books_replica_.emplace(location_uid, make_book(location_uid));
  }
  return books_replica_.at(location_uid);
}

void Bookkeeper::try_sync_position_end(const PositionEnd &position_end) {
  if (not sync_position_ or not app_.has_location(position_end.holder_uid)) {
    return;
  }

  auto old_book = get_book(position_end.holder_uid);
  auto new_book = get_book_replica(position_end.holder_uid);
  bool position_changed = false;
  auto position_compare = [](const PositionMap &source_map, Book_ptr &target_book) {
    bool changed = false;
    for (auto &source_pair : source_map) {
      auto &source_position = source_pair.second;
      const auto &target_position = target_book->get_position_for(source_position.direction, source_position);
      changed |= source_position.volume != target_position.volume;                     // 数量
      changed |= source_position.yesterday_volume != target_position.yesterday_volume; // 昨仓数量
    }
    return changed;
  };

  /// 用new_book的position去检测old_book的position,new有old无会加上
  position_changed |= position_compare(new_book->long_positions, old_book);
  position_changed |= position_compare(new_book->short_positions, old_book);
  /// 用old_book的position去检测new_book的position，old有new无会设置为0删掉
  position_changed |= position_compare(old_book->long_positions, new_book);
  position_changed |= position_compare(old_book->short_positions, new_book);

  if (position_changed) {
    for (auto &book_listener : book_listeners_) {
      book_listener->on_position_sync_reset(*old_book, *new_book);
    }
    old_book->mirror_position_from(*new_book);
  }
  books_replica_.erase(position_end.holder_uid); // delete replica every time
}

void Bookkeeper::add_book_listener(const BookListener_ptr &book_listener) { book_listeners_.push_back(book_listener); }

void Bookkeeper::mirror_positions(int64_t trigger_time, uint32_t strategy_uid) {
  auto strategy_book = get_book(strategy_uid);

  auto reset_positions = [trigger_time](auto &positions) {
    for (auto &item : positions) {
      auto &position = item.second;
      position.volume = 0;
      position.yesterday_volume = 0;
      position.frozen_total = 0;
      position.frozen_yesterday = 0;
      position.avg_open_price = 0;
      position.position_cost_price = 0;
      position.update_time = trigger_time;
    }
  };
  reset_positions(strategy_book->long_positions);
  reset_positions(strategy_book->short_positions);

  auto copy_positions = [&](const auto &positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      if (strategy_book->has_position_for(position)) {
        auto &strategy_position = strategy_book->get_position_for(position.direction, position);

        auto volume = strategy_position.volume;
        auto yesterday_volume = strategy_position.yesterday_volume;
        auto frozen_total = strategy_position.frozen_total;
        auto frozen_yesterday = strategy_position.frozen_yesterday;
        auto avg_open_price = strategy_position.avg_open_price;
        auto position_cost_price = strategy_position.position_cost_price;

        auto total_volume = strategy_position.volume + position.volume;

        longfist::copy(strategy_position, position);
        strategy_position.holder_uid = strategy_uid;
        strategy_position.ledger_category = LedgerCategory::Strategy;
        strategy_position.update_time = trigger_time;

        if (volume > 0) {
          strategy_position.volume += volume;
          strategy_position.yesterday_volume += yesterday_volume;
          strategy_position.frozen_total += frozen_total;
          strategy_position.frozen_yesterday += frozen_yesterday;
          strategy_position.avg_open_price =
              ((position.avg_open_price * position.volume) + (avg_open_price * volume)) / total_volume;
          strategy_position.position_cost_price =
              ((position.position_cost_price * position.volume) + (position_cost_price * volume)) / total_volume;
        }
      }
    }
  };
  for (const auto &pair : get_books()) {
    auto &book = pair.second;
    auto holder_uid = book->asset.holder_uid;
    if (book->asset.ledger_category == LedgerCategory::Account and app_.has_channel(strategy_uid, holder_uid)) {
      copy_positions(book->long_positions);
      copy_positions(book->short_positions);
    }
  }
  strategy_book->update(trigger_time);
}

} // namespace kungfu::wingchun::book