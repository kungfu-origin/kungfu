// SPDX-License-Identifier: Apache-2.0

//
// Created by qlu on 2019/1/16.
//

#ifndef WINGCHUN_STRATEGY_H
#define WINGCHUN_STRATEGY_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::strategy {
FORWARD_DECLARE_CLASS_PTR(Context)

class Strategy {
public:
  virtual ~Strategy() = default;

  // 运行前
  virtual void pre_start(Context_ptr &context){};

  virtual void post_start(Context_ptr &context){};

  // 退出前
  virtual void pre_stop(Context_ptr &context){};

  virtual void post_stop(Context_ptr &context){};

  // 交易日切换回调
  //@param daytime 交易日
  virtual void on_trading_day(Context_ptr &context, int64_t daytime){};

  // 行情数据更新回调
  // @param quote             行情数据
  // @param location          数据来源
  virtual void on_quote(Context_ptr &context, const longfist::types::Quote &quote,
                        const kungfu::yijinjing::data::location_ptr &location){};

  // 行情数据更新回调
  // @param tree              行情数据
  // @param location          数据来源
  virtual void on_tree(Context_ptr &context, const longfist::types::Tree &tree,
                       const kungfu::yijinjing::data::location_ptr &location){};

  // bar 数据更新回调
  // @param bar               bar 数据
  // @param location          数据来源
  virtual void on_bar(Context_ptr &context, const longfist::types::Bar &bar,
                      const kungfu::yijinjing::data::location_ptr &location){};

  // 逐笔委托更新回调
  // @param entrust           逐笔委托数据
  // @param location          数据来源
  virtual void on_entrust(Context_ptr &context, const longfist::types::Entrust &entrust,
                          const kungfu::yijinjing::data::location_ptr &location){};

  // 逐笔成交更新回调
  // @param transaction       逐笔成交数据
  // @param location          数据来源
  virtual void on_transaction(Context_ptr &context, const longfist::types::Transaction &transaction,
                              const kungfu::yijinjing::data::location_ptr &location){};

  // 订单信息更新回调
  // @param order             订单信息数据
  // @param location          数据来源
  virtual void on_order(Context_ptr &context, const longfist::types::Order &order,
                        const kungfu::yijinjing::data::location_ptr &location){};

  // 订单操作错误回调
  // @param order             订单信息数据
  // @param location          数据来源
  virtual void on_order_action_error(Context_ptr &context, const longfist::types::OrderActionError &error,
                                     const kungfu::yijinjing::data::location_ptr &location){};

  // 订单成交回报回调
  // @param trade             订单成交数据
  // @param location          数据来源
  virtual void on_trade(Context_ptr &context, const longfist::types::Trade &trade,
                        const kungfu::yijinjing::data::location_ptr &location){};

  // 历史订单回报回调
  // @param history_order     历史订单数据
  // @param location          数据来源
  virtual void on_history_order(Context_ptr &context, const longfist::types::HistoryOrder &history_order,
                                const kungfu::yijinjing::data::location_ptr &location){};

  // 历史订单成交回报回调
  // @param history_order     历史订单成交数据
  // @param location          数据来源
  virtual void on_history_trade(Context_ptr &context, const longfist::types::HistoryTrade &history_trade,
                                const kungfu::yijinjing::data::location_ptr &location){};
  // 历史订单查询报错回调
  // @param error              报错信息
  // @param location          数据来源
  virtual void on_req_history_order_error(Context_ptr &context, const longfist::types::RequestHistoryOrderError &error,
                                          const kungfu::yijinjing::data::location_ptr &location){};

  // 历史成交查询报错回调
  // @param error              报错信息
  // @param location          数据来源
  virtual void on_req_history_trade_error(Context_ptr &context, const longfist::types::RequestHistoryTradeError &error,
                                          const kungfu::yijinjing::data::location_ptr &location){};

  // 同步柜台资金持仓信息回调
  // @param old_book          更新前本地维护的旧数据
  // @param new_book          更新后重新从柜台获取的新数据
  virtual void on_position_sync_reset(Context_ptr &context, const kungfu::wingchun::book::Book &old_book,
                                      const kungfu::wingchun::book::Book &new_book){};

  // 同步柜台资金信息回调
  // @param old_asset         更新前本地维护的旧数据
  // @param new_asset         更新后重新从柜台获取的新数据
  virtual void on_asset_sync_reset(Context_ptr &context, const kungfu::longfist::types::Asset &old_asset,
                                   const kungfu::longfist::types::Asset &new_asset){};

  // 同步柜台资金信息回调
  // @param old_asset_margin         更新前本地维护的旧数据
  // @param new_asset_margin         更新后重新从柜台获取的新数据
  virtual void on_asset_margin_sync_reset(Context_ptr &context,
                                          const kungfu::longfist::types::AssetMargin &old_asset_margin,
                                          const kungfu::longfist::types::AssetMargin &new_asset_margin){};

  // 断开回调
  // @param deregister     断开数据
  // @param location          数据来源
  virtual void on_deregister(Context_ptr &context, const longfist::types::Deregister &deregister,
                             const kungfu::yijinjing::data::location_ptr &location){};

  // 客户端状态变化回调
  // @param brokerStateUpdate     状态变化
  // @param location          数据来源
  virtual void on_broker_state_change(Context_ptr &context,
                                      const longfist::types::BrokerStateUpdate &broker_state_update,
                                      const kungfu::yijinjing::data::location_ptr &location){};

  /**
   * 自定义数据回调, 如果数据的msg_type不在AllTypes, 则会通过此函数响应
   * @param context
   * @param msg_type 数据类型
   * @param data  自定义数据, 以字节数组表示
   * @param length 自定义数据的字节数
   * @param location 数据来源
   */
  virtual void on_custom_data(Context_ptr &context, uint32_t msg_type, const std::vector<uint8_t> &data,
                              uint32_t length, const kungfu::yijinjing::data::location_ptr &location){};
};

DECLARE_PTR(Strategy)
} // namespace kungfu::wingchun::strategy
#endif // WINGCHUN_STRATEGY_H
