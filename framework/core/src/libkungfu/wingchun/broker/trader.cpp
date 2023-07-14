// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/common.h>
#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/yijinjing/journal/assemble.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;

namespace kungfu::wingchun::broker {
TraderVendor::TraderVendor(locator_ptr locator, const std::string &group, const std::string &name, bool low_latency)
    : BrokerVendor(location::make_shared(mode::LIVE, category::TD, group, name, std::move(locator)), low_latency) {}

void TraderVendor::set_service(Trader_ptr service) { service_ = std::move(service); }

void TraderVendor::react() {
  events_ | skip_until(events_ | is(RequestStart::tag)) | is(OrderInput::tag) | $$(service_->handle_order_input(event));
  events_ | skip_until(events_ | is(RequestStart::tag)) | is_custom() | $$(service_->on_custom_event(event));
  apprentice::react();
}

void TraderVendor::on_react() {
  events_ | is(ResetBookRequest::tag) |
      $([&](const event_ptr &event) { get_writer(location::PUBLIC)->mark(now(), ResetBookRequest::tag); });
}

void TraderVendor::on_start() {
  BrokerVendor::on_start();

  events_ | is(BlockMessage::tag) | $$(service_->insert_block_message(event));
  events_ | is(OrderAction::tag) | $$(service_->cancel_order(event));
  events_ | is(AssetRequest::tag) | $$(service_->req_account());
  events_ | is(OrderTradeRequest::tag) | $$(service_->req_order_trade());
  events_ | is(Deregister::tag) | $$(service_->on_strategy_exit(event));
  events_ | is(TimeKeyValue::tag) | $$(service_->on_time_key_value(event));
  events_ | is(PositionRequest::tag) | $$(service_->req_position());
  events_ | is(RequestHistoryOrder::tag) | $$(service_->req_history_order(event));
  events_ | is(RequestHistoryTrade::tag) | $$(service_->req_history_trade(event));
  events_ | is(AssetSync::tag) | $$(service_->handle_asset_sync());
  events_ | is(PositionSync::tag) | $$(service_->handle_position_sync());

  events_ | filter([&](const event_ptr &event) {
    return event->msg_type() == BatchOrderBegin::tag or event->msg_type() == BatchOrderEnd::tag;
  }) | $$(service_->handle_batch_order_tag(event));

  service_->recover();
  service_->on_recover();
  service_->on_start();
}

BrokerService_ptr TraderVendor::get_service() { return service_; }

void TraderVendor::clean_orders() {
  std::set<uint32_t> strategy_uids = {};
  auto master_cmd_writer = get_writer(get_master_commands_uid());
  for (auto &pair : state_bank_[boost::hana::type_c<Order>]) {
    auto &order_state = pair.second;
    auto &order = const_cast<Order &>(order_state.data);
    auto strategy_uid = order_state.dest;
    if (order.status == OrderStatus::Submitted or order.status == OrderStatus::Pending or
        order.status == OrderStatus::PartialFilledActive) {

      order.status = OrderStatus::Lost;
      order.update_time = time::now_in_nano();

      if (strategy_uid == location::PUBLIC) {
        write_to(now(), order);
        continue;
      }

      strategy_uids.emplace(strategy_uid);

      events_ | is(Channel::tag) | filter([&, strategy_uid](const event_ptr &event) {
        const Channel &channel = event->data<Channel>();
        return channel.source_id == get_home_uid() and channel.dest_id == strategy_uid;
      }) | first() |
          $([this, order, strategy_uid](auto event) { write_to(now(), order, strategy_uid); });
    }
  }
  for (auto uid : strategy_uids) {
    if (not has_writer(uid)) {
      request_write_to(now(), uid);
    }
  }
}

void TraderVendor::on_trading_day(const event_ptr &event, int64_t daytime) { service_->on_trading_day(event, daytime); }

[[maybe_unused]] const std::string &Trader::get_account_id() const { return get_home()->name; }

yijinjing::journal::writer_ptr Trader::get_asset_writer() const {
  return get_writer(sync_asset_ ? location::SYNC : location::PUBLIC);
}

yijinjing::journal::writer_ptr Trader::get_asset_margin_writer() const {
  return get_writer(sync_asset_margin_ ? location::SYNC : location::PUBLIC);
}

yijinjing::journal::writer_ptr Trader::get_position_writer() const {
  return get_writer(sync_position_ ? location::SYNC : location::PUBLIC);
}

void Trader::enable_asset_sync() { sync_asset_ = true; }

void Trader::enable_asset_margin_sync() { sync_asset_margin_ = true; }

void Trader::enable_positions_sync() { sync_position_ = true; }

bool Trader::write_default_asset_margin() {
  SPDLOG_INFO("Write an empty AssetMargin by default");
  sync_asset_margin_ = true;
  auto writer = get_asset_margin_writer();
  AssetMargin &asset_margin = writer->open_data<AssetMargin>();
  asset_margin.holder_uid = get_home_uid();
  asset_margin.update_time = yijinjing::time::now_in_nano();
  writer->close_data();
  return false;
}

void Trader::handle_asset_sync() {
  if (state_ == BrokerState::Ready) {
    req_account();
    write_default_asset_margin();
  }
}

void Trader::handle_position_sync() {
  if (state_ == BrokerState::Ready) {
    req_position();
  }
}

bool Trader::has_self_deal_risk(const event_ptr &event) {
  if (not self_deal_detect_) {
    return false;
  }
  const OrderInput &input = event->data<OrderInput>();
  static std::string str_ex_instrument;
  str_ex_instrument = input.exchange_id.to_string() + input.instrument_id.to_string();
  auto risk_check = [&]() -> bool {
    auto iter = map_ex_instrument_to_order_ids_.find(str_ex_instrument);

    /// 没有相同的标的, 判定为不存在风险
    if (iter == map_ex_instrument_to_order_ids_.end()) {
      return false;
    }

    /// 存在相同标的, 遍历每一个order_id, 判断是否存在自成交风险
    return std::any_of(iter->second.begin(), iter->second.end(), [&](const auto order_id) -> bool {
      const auto order_iter = orders_.find(order_id);

      /// 只接收到了OrderInput, 没有生成相应的order, 判定为不存在风险
      if (order_iter == orders_.end()) {
        return false;
      }

      const Order &order = order_iter->second.data;

      /// 方向相同或者是委托完结, 判定为不存在风险
      if (order.side == input.side or is_final_status(order.status)) {
        return false;
      }

      /// 存在反方向未完成委托, 且当前委托是市价, 判定为存在风险
      if (input.price_type != PriceType::Limit) {
        return true;
      }

      /// 限价, 判断买卖方向和价格
      if (input.side == Side::Buy and input.limit_price < order.limit_price) {
        return false; /// 新委托买价低于已存在卖价, 判定为不存在风险
      } else if (input.side == Side::Sell and input.limit_price > order.limit_price) {
        return false; /// 新委托卖价高于已存在买价, 判定为不存在风险
      } else {
        return true; /// 新委托买价大于等于已存在卖价, 或 新委托卖价小于等于已存在买价, 判定为存在风险
      }
    });
  };

  if (risk_check()) {
    return true;
  }
  map_ex_instrument_to_order_ids_.try_emplace(str_ex_instrument).first->second.emplace(input.order_id);
  return false;
}

void Trader::handle_order_input(const event_ptr &event) {
  if (has_self_deal_risk(event)) {
    Order &order = get_writer(event->source())->open_data<Order>();
    order_from_input(event->data<OrderInput>(), order);
    order.status = OrderStatus::Error;
    strncpy(order.error_msg, "该委托存在自成交风险,已拒绝下单", ERROR_MSG_LEN);
    order.insert_time = event->gen_time();
    order.update_time = event->gen_time();
    get_writer(event->source())->close_data();
    return;
  }

  /// try_emplace default insert false to map, means not batch mode
  if (batch_status_.try_emplace(event->source()).first->second) {
    const OrderInput &input = event->data<OrderInput>();
    order_inputs_.try_emplace(event->source()).first->second.push_back(input);
  } else {
    insert_order(event);
  }
}

void Trader::handle_batch_order_tag(const event_ptr &event) {
  if (event->msg_type() == BatchOrderBegin::tag) {
    batch_status_.insert_or_assign(event->source(), true);
  } else if (event->msg_type() == BatchOrderEnd::tag) {
    batch_status_.insert_or_assign(event->source(), false);
    insert_batch_orders(event);
    clear_order_inputs(event->source());
  }
}

bool Trader::insert_block_message(const event_ptr &event) {
  const BlockMessage &msg = event->data<BlockMessage>();
  return block_messages_.try_emplace(msg.block_id, msg).second;
}

void Trader::enable_self_detect() { self_deal_detect_ = true; }

void Trader::recover() {
  if (disable_recover_) {
    return;
  }

  deal_write_frame();
  deal_read_frame();
}

void Trader::deal_write_frame() {
  assemble asb_write(get_home(), location::PUBLIC, AssembleMode::Write);
  asb_write.seek_to_time(time::today_start()); // recover from today
  SPDLOG_DEBUG("before assemble read");
  int64_t count = 0;
  while (asb_write.data_available()) {
    const auto &frame = asb_write.current_frame();
    if (frame->msg_type() == Order::tag) {
      const Order &order = frame->data<Order>();
      orders_.insert_or_assign(order.order_id, state<Order>(frame->source(), frame->dest(), frame->gen_time(), order));
      map_ex_instrument_to_order_ids_.try_emplace(order.exchange_id.to_string() + order.instrument_id.to_string())
          .first->second.emplace(order.order_id);
    } else if (frame->msg_type() == Trade::tag) {
      const Trade &trade = frame->data<Trade>();
      trades_.insert_or_assign(trade.trade_id, state<Trade>(frame->source(), frame->dest(), frame->gen_time(), trade));
    }
    asb_write.next();
    ++count;
  }
  SPDLOG_DEBUG("after assemble read, count: {}", count);

  // set order as Lost which without external_order_id
  for (auto &pair : orders_) {
    Order &order = pair.second.data;
    if (not is_final_status(order.status) and (order.external_order_id.to_string().empty() or disable_recover_)) {
      order.status = OrderStatus::Lost;
      order.update_time = time::now_in_nano();

      if (has_writer(pair.second.dest)) {
        write_to(order, pair.second.dest);
      }
    }
  }
}

void Trader::deal_read_frame() {
  // write a Lost Order to journal when read an OrderInput whose order_id not in orders_
  assemble asb_read(get_home(), get_home_uid(), AssembleMode::Read);
  asb_read.disjoin(get_vendor().get_ledger_home_location()->location_uid); // ledger
  asb_read.disjoin(get_vendor().get_master_home_location()->location_uid); // master
  asb_read.seek_to_time(time::today_start());                              // recover from today
  SPDLOG_DEBUG("before assemble read");
  int64_t count = 0;
  while (asb_read.data_available()) {
    const auto &frame = asb_read.current_frame();
    if (frame->msg_type() == OrderInput::tag) {
      const OrderInput &order_input = frame->data<OrderInput>();
      if (orders_.find(order_input.order_id) == orders_.end()) {
        if (has_writer(frame->dest())) {
          Order &order = get_writer(frame->dest())->open_data<Order>();
          order_from_input(order_input, order);
          order.status = OrderStatus::Lost;
          order.update_time = time::now_in_nano();
          get_writer(frame->dest())->close_data();
        }
      }
    }
    asb_read.next();
    ++count;
  }
  SPDLOG_DEBUG("after assemble read, count: {}", count);
}

void Trader::clear_order_inputs(const uint64_t location_uid) { order_inputs_.erase(location_uid); }

[[maybe_unused]] void Trader::disable_recover() { disable_recover_ = true; }

} // namespace kungfu::wingchun::broker
