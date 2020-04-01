//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_CONTEXT_H
#define WINGCHUN_CONTEXT_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>

#include <kungfu/wingchun/algo/algo.h>
#include <kungfu/wingchun/book/book.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/strategy/strategy.h>

namespace kungfu::wingchun::strategy {
class Runner;

class Context : public std::enable_shared_from_this<Context> {
public:
  explicit Context(yijinjing::practice::apprentice &app, const rx::connectable_observable<event_ptr> &events);

  ~Context() = default;

  //获取当前纳秒时间
  //@return            当前纳秒时间
  int64_t now() const;

  virtual void add_timer(int64_t nanotime, const std::function<void(event_ptr)> &callback);

  virtual void add_time_interval(int64_t duration, const std::function<void(event_ptr)> &callback);

  //添加策略使用的交易账户
  //@param source_id   柜台ID
  //@param account_id  账户ID
  //@param cash_limit  可用资金限制
  virtual void add_account(const std::string &source, const std::string &account, double cash_limit);

  //获取交易账户列表
  virtual const std::unordered_map<uint32_t, yijinjing::data::location_ptr> &list_accounts() const;

  //获取交易账户可用资金限制
  //@param account_id  账户ID
  virtual double get_account_cash_limit(const std::string &account);

  //订阅行情
  //@param source_id   柜台ID
  //@param instruments 合约列表
  //@param exchange_id 交易所ID
  virtual void subscribe(const std::string &source, const std::vector<std::string> &instruments,
                         const std::string &exchange);

  virtual void subscribe_all(const std::string &source);

  //报单
  //@param instrument_id 合约ID
  //@param exchange_id   交易所ID
  //@param account_id    账户ID
  //@param limit_price   价格
  //@param volume        数量
  //@param type          报单价格类型
  //@param side          买卖方向
  //@param offset        开平方向
  //@return              订单ID
  virtual uint64_t insert_order(const std::string &instrument_id, const std::string &exchange_id,
                                const std::string &account, double limit_price, int64_t volume,
                                longfist::enums::PriceType type, longfist::enums::Side side,
                                longfist::enums::Offset offset = longfist::enums::Offset::Open,
                                longfist::enums::HedgeFlag hedge_flag = longfist::enums::HedgeFlag::Speculation);

  //撤单
  //@param order_id      订单ID
  //@return              撤单操作ID
  virtual uint64_t cancel_order(uint64_t order_id);

  int64_t get_trading_day() { return app_.get_trading_day(); }

  book::Bookkeeper &get_bookkeeper() { return bookkeeper_; }

  virtual algo::AlgoContext_ptr get_algo_context() const { return algo_context_; }

protected:
  virtual void on_start();

  yijinjing::practice::apprentice &app_;
  const rx::connectable_observable<event_ptr> &events_;
  const std::unordered_map<uint64_t, state<longfist::types::Quote>> &quotes_;

private:
  broker::ManualClient broker_client_;
  book::Bookkeeper bookkeeper_;
  algo::AlgoContext_ptr algo_context_;
  yijinjing::data::location_ptr ledger_location_;
  std::unordered_map<uint32_t, uint32_t> account_location_ids_;
  std::unordered_map<uint32_t, yijinjing::data::location_ptr> accounts_;
  std::unordered_map<uint32_t, double> account_cash_limits_;
  std::unordered_map<std::string, yijinjing::data::location_ptr> market_data_;

  uint32_t lookup_account_location_id(const std::string &account);

  const yijinjing::data::location_ptr &find_marketdata(const std::string &source);

  template <class DataType> bool is_subscribed(const event_ptr &event) {
    const DataType &data = event->data<DataType>();
    return broker_client_.is_subscribed(event->source(), data.exchange_id, data.instrument_id);
  }

  friend class Runner;
};
} // namespace kungfu::wingchun::strategy

#endif // WINGCHUN_CONTEXT_H
