//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_ENUM_H
#define KUNGFU_LONGFIST_ENUM_H

#include <fmt/ostream.h>
#include <spdlog/fmt/ostr.h>

namespace kungfu::longfist::enums {
enum class mode : int8_t { LIVE, DATA, REPLAY, BACKTEST };

inline std::ostream &operator<<(std::ostream &os, mode t) { return os << int8_t(t); }

inline std::string get_mode_name(mode m) {
  switch (m) {
  case mode::LIVE:
    return "live";
  case mode::DATA:
    return "data";
  case mode::REPLAY:
    return "replay";
  case mode::BACKTEST:
    return "backtest";
  default:
    return "live";
  }
}

inline mode get_mode_by_name(const std::string &name) {
  if (name == "live")
    return mode::LIVE;
  else if (name == "data")
    return mode::DATA;
  else if (name == "replay")
    return mode::REPLAY;
  else if (name == "backtest")
    return mode::BACKTEST;
  else
    return mode::LIVE;
}

enum class category : int8_t { MD, TD, STRATEGY, SYSTEM };

inline std::ostream &operator<<(std::ostream &os, category t) { return os << int8_t(t); }

inline std::string get_category_name(category c) {
  switch (c) {
  case category::MD:
    return "md";
  case category::TD:
    return "td";
  case category::STRATEGY:
    return "strategy";
  case category::SYSTEM:
    return "system";
  default:
    return "system";
  }
}

inline category get_category_by_name(const std::string &name) {
  if (name == "md")
    return category::MD;
  else if (name == "td")
    return category::TD;
  else if (name == "strategy")
    return category::STRATEGY;
  else
    return category::SYSTEM;
}

enum class layout : int8_t { JOURNAL, SQLITE, NANOMSG, LOG };

inline std::string get_layout_name(layout l) {
  switch (l) {
  case layout::JOURNAL:
    return "journal";
  case layout::SQLITE:
    return "db";
  case layout::NANOMSG:
    return "nn";
  case layout::LOG:
    return "log";
  default:
    return "log";
  }
}

enum class InstrumentType : int8_t {
  Unknown,     //未知
  Stock,       //普通股票
  Future,      //期货
  Bond,        //债券
  StockOption, //股票期权
  Fund,        //基金
  Index,       //指数
  Repo,        //回购
  Warrant,     //认权证
  Iopt,        //牛熊证
  Crypto,      //数字货币
};

inline std::ostream &operator<<(std::ostream &os, InstrumentType t) { return os << int8_t(t); }

enum class ExecType : int8_t { Unknown, Cancel, Trade };

inline std::ostream &operator<<(std::ostream &os, ExecType t) { return os << int8_t(t); }

enum class BsFlag : int8_t { Unknown, Buy, Sell };

inline std::ostream &operator<<(std::ostream &os, BsFlag t) { return os << int8_t(t); }

enum class Side : int8_t {
  Buy,                       //买入
  Sell,                      //卖出
  Lock,                      //锁仓
  Unlock,                    //解锁
  Exec,                      //行权
  Drop,                      //放弃行权
  Purchase,                  //申购
  Redemption,                //赎回
  Split,                     //拆分
  Merge,                     //合并
  MarginTrade,               //融资买入
  ShortSell,                 //融券卖出
  RepayMargin,               //卖券还款
  RepayStock,                //买券还券
  CashRepayMargin,           //现金还款
  StockRepayStock,           //现券还券
  SurplusStockTransfer,      //余券划转
  GuaranteeStockTransferIn,  //担保品转入
  GuaranteeStockTransferOut, //担保品转出
  Unknown = 99
};

inline std::ostream &operator<<(std::ostream &os, Side t) { return os << int8_t(t); }

enum class Offset : int8_t { Open, Close, CloseToday, CloseYesterday };

inline std::ostream &operator<<(std::ostream &os, Offset t) { return os << int8_t(t); }

enum class HedgeFlag : int8_t { Speculation, Arbitrage, Hedge, Covered };

inline std::ostream &operator<<(std::ostream &os, HedgeFlag t) { return os << int8_t(t); }

enum class OrderActionFlag : int8_t {
  Cancel,
};

inline std::ostream &operator<<(std::ostream &os, OrderActionFlag t) { return os << int8_t(t); }

enum class PriceType : int8_t {
  Limit, //限价,证券通用
  Any, //市价，证券通用，对于股票上海为最优五档剩余撤销，深圳为即时成交剩余撤销，建议客户采用
  FakBest5,    //上海深圳最优五档即时成交剩余撤销，不需要报价
  ForwardBest, //深圳本方方最优价格申报, 不需要报价
  ReverseBest, //上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报，不需要报价
  Fak,         //深圳即时成交剩余撤销，不需要报价
  Fok,         //深圳市价全额成交或者撤销，不需要报价
  UnKnown
};

inline std::ostream &operator<<(std::ostream &os, PriceType t) { return os << int8_t(t); }

enum class VolumeCondition : int8_t { Any, Min, All };

inline std::ostream &operator<<(std::ostream &os, VolumeCondition t) { return os << int8_t(t); }

enum class TimeCondition : int8_t { IOC, GFD, GTC };

inline std::ostream &operator<<(std::ostream &os, TimeCondition t) { return os << int8_t(t); }

enum class OrderStatus : int8_t {
  Unknown,
  Submitted,
  Pending,
  Cancelled,
  Error,
  Filled,
  PartialFilledNotActive,
  PartialFilledActive,
  Lost
};

inline std::ostream &operator<<(std::ostream &os, OrderStatus t) { return os << int8_t(t); }

enum class Direction : int8_t { Long, Short };

inline std::ostream &operator<<(std::ostream &os, Direction t) { return os << int8_t(t); }

enum class AccountType : int8_t { Stock, Credit, Future };

inline std::ostream &operator<<(std::ostream &os, AccountType t) { return os << int8_t(t); }

enum class CommissionRateMode : int8_t { ByAmount, ByVolume };

inline std::ostream &operator<<(std::ostream &os, CommissionRateMode t) { return os << int8_t(t); }

enum class LedgerCategory : int8_t {
  Account,
  Strategy,
};

inline std::ostream &operator<<(std::ostream &os, LedgerCategory t) { return os << int8_t(t); }

enum class BrokerState : int8_t {
  Pending = 0,
  Idle = 1,
  DisConnected = 2,
  Connected = 3,
  LoggedIn = 4,
  LoginFailed = 5,
  Ready = 100
};

inline std::ostream &operator<<(std::ostream &os, BrokerState t) { return os << int8_t(t); }

enum class StrategyState : int8_t { Normal, Warn, Error };

inline std::ostream &operator<<(std::ostream &os, StrategyState t) { return os << int8_t(t); }
//权限订阅数据类型
enum class MarketType : uint8_t {
  All,   ///< 表示全市场
  BSE,   ///< 北交所
  SHFE,  ///< 上期所
  CFFEX, ///< 中金所
  DCE,   ///< 大商所
  CZCE,  ///< 郑商所
  INE,   ///< 上期能源
  SSE,   ///< 上交所
  SZSE,  ///< 深交所
  HKEx ///< 港交所(暂时不支持直连港交所, 港交所行情数据通过深交所和上交所的港股通获取, 市场类型为kSZSE/kSSE)
};

//证券数据类型
enum class SubscribeDataType : uint64_t {
  All = 0x000000000000,         ///< 订阅全部证券数据类别
  Snapshot = 0x000000000001,    ///< 订阅快照数据类别
  Entrust = 0x000000000002,     ///< 订阅逐笔委托数据
  Transaction = 0x000000000004, ///< 订阅逐笔成交数据
};

//证券品种类型
enum class SubscribeInstrumentType : uint64_t {
  All = 0x000000000000,         ///< 订阅全部证券品种类别
  Stock = 0x000000000001,       ///< 订阅股票证券品种类别
  Future = 0x000000000002,      ///< 订阅期货证券品种类别
  Bond = 0x000000000004,        ///< 订阅债券证券品种类别
  StockOption = 0x00000000008,  ///< 订阅期权证券品种类别
  FutureOption = 0x00000000010, ///< 订阅期货期权品种类别
  Fund = 0x0000000000020,       ///< 订阅基金证券品种类别
  Index = 0x000000000040,       ///< 订阅指数证券品种类别
  HKT = 0x000000000080,         ///< 订阅港股通证券品种类别
};

template <typename T, typename U> inline T sub_data_bitwise(const T &a, const T &b) {
  return static_cast<T>(static_cast<U>(a) | static_cast<U>(b));
}
} // namespace kungfu::longfist::enums
#endif // KUNGFU_LONGFIST_ENUM_H
