// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_ENUM_H
#define KUNGFU_LONGFIST_ENUM_H

#include <fmt/ostream.h>
#include <spdlog/fmt/ostr.h>

namespace kungfu::longfist::enums {
enum class mode : int8_t { LIVE, DATA, REPLAY, BACKTEST };

NLOHMANN_JSON_SERIALIZE_ENUM(mode, {
                                       {mode::LIVE, "LIVE"},
                                       {mode::DATA, "DATA"},
                                       {mode::REPLAY, "REPLAY"},
                                       {mode::BACKTEST, "BACKTEST"},
                                   })

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

NLOHMANN_JSON_SERIALIZE_ENUM(category, {
                                           {category::MD, "MD"},
                                           {category::TD, "TD"},
                                           {category::STRATEGY, "STRATEGY"},
                                           {category::SYSTEM, "SYSTEM"},
                                       })

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

NLOHMANN_JSON_SERIALIZE_ENUM(layout, {
                                         {layout::JOURNAL, "JOURNAL"},
                                         {layout::SQLITE, "SQLITE"},
                                         {layout::NANOMSG, "NANOMSG"},
                                         {layout::LOG, "LOG"},
                                     })

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

// 权限订阅数据类型
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

NLOHMANN_JSON_SERIALIZE_ENUM(MarketType, {
                                             {MarketType::All, "All"},
                                             {MarketType::BSE, "BSE"},
                                             {MarketType::SHFE, "SHFE"},
                                             {MarketType::CFFEX, "CFFEX"},
                                             {MarketType::DCE, "DCE"},
                                             {MarketType::CZCE, "CZCE"},
                                             {MarketType::INE, "INE"},
                                             {MarketType::SSE, "SSE"},
                                             {MarketType::SZSE, "SZSE"},
                                             {MarketType::HKEx, "HKEx"},
                                         })

// 证券数据类型
enum class SubscribeDataType : uint64_t {
  All = 0x000000000000,         ///< 订阅全部证券数据类别
  Snapshot = 0x000000000001,    ///< 订阅快照数据类别
  Entrust = 0x000000000002,     ///< 订阅逐笔委托数据
  Transaction = 0x000000000004, ///< 订阅逐笔成交数据
};

NLOHMANN_JSON_SERIALIZE_ENUM(SubscribeDataType, {
                                                    {SubscribeDataType::All, "All"},
                                                    {SubscribeDataType::Snapshot, "Snapshot"},
                                                    {SubscribeDataType::Entrust, "Entrust"},
                                                    {SubscribeDataType::Transaction, "Transaction"},
                                                })

// for subscribe
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

  /// SubscribeInstrumentType(All | Stock), 虽然没有定义值为3的枚举值, 这个操作不会报错, 会得到一个值为3的枚举值
};

NLOHMANN_JSON_SERIALIZE_ENUM(SubscribeInstrumentType, {
                                                          {SubscribeInstrumentType::All, "All"},
                                                          {SubscribeInstrumentType::Stock, "Stock"},
                                                          {SubscribeInstrumentType::Future, "Future"},
                                                          {SubscribeInstrumentType::Bond, "Bond"},
                                                          {SubscribeInstrumentType::StockOption, "StockOption"},
                                                          {SubscribeInstrumentType::FutureOption, "FutureOption"},
                                                          {SubscribeInstrumentType::Fund, "Fund"},
                                                          {SubscribeInstrumentType::Index, "Index"},
                                                          {SubscribeInstrumentType::HKT, "HKT"},
                                                      })

// for trading, different type has different minimum volume, price, accounting rules for making order
enum class InstrumentType : int8_t {
  Unknown,     // 未知
  Stock,       // 股票
  Future,      // 期货
  Bond,        // 债券
  StockOption, // 股票期权
  TechStock,   // 科技股
  Fund,        // 基金
  Index,       // 指数
  Repo,        // 回购
  Warrant,     // 认权证
  Iopt,        // 牛熊证
  Crypto,      // 数字货币
};

NLOHMANN_JSON_SERIALIZE_ENUM(InstrumentType, {
                                                 {InstrumentType::Unknown, "Unknown"},
                                                 {InstrumentType::Stock, "Stock"},
                                                 {InstrumentType::StockOption, "StockOption"},
                                                 {InstrumentType::TechStock, "TechStock"},
                                                 {InstrumentType::Future, "Future"},
                                                 {InstrumentType::Bond, "Bond"},
                                                 {InstrumentType::Fund, "Fund"},
                                                 {InstrumentType::Index, "Index"},
                                                 {InstrumentType::Repo, "Repo"},
                                                 {InstrumentType::Crypto, "Crypto"},
                                                 {InstrumentType::Warrant, "Warrant"},
                                                 {InstrumentType::Iopt, "Iopt"},
                                             })

inline std::ostream &operator<<(std::ostream &os, InstrumentType t) { return os << int8_t(t); }

enum class ExecType : int8_t { Unknown, Cancel, Trade };

NLOHMANN_JSON_SERIALIZE_ENUM(ExecType, {
                                           {ExecType::Unknown, "Unknown"},
                                           {ExecType::Cancel, "Cancel"},
                                           {ExecType::Trade, "Trade"},
                                       })

inline std::ostream &operator<<(std::ostream &os, ExecType t) { return os << int8_t(t); }

enum class BsFlag : int8_t { Unknown, Buy, Sell };

NLOHMANN_JSON_SERIALIZE_ENUM(BsFlag, {
                                         {BsFlag::Unknown, "Unknown"},
                                         {BsFlag::Buy, "Buy"},
                                         {BsFlag::Sell, "Sell"},
                                     })

inline std::ostream &operator<<(std::ostream &os, BsFlag t) { return os << int8_t(t); }

enum class Side : int8_t {
  Buy,                       // 买入
  Sell,                      // 卖出
  Lock,                      // 锁仓
  Unlock,                    // 解锁
  Exec,                      // 行权
  Drop,                      // 放弃行权
  Purchase,                  // 申购
  Redemption,                // 赎回
  Split,                     // 拆分
  Merge,                     // 合并
  MarginTrade,               // 融资买入
  ShortSell,                 // 融券卖出
  RepayMargin,               // 卖券还款
  RepayStock,                // 买券还券
  CashRepayMargin,           // 现金还款
  StockRepayStock,           // 现券还券
  SurplusStockTransfer,      // 余券划转
  GuaranteeStockTransferIn,  // 担保品转入
  GuaranteeStockTransferOut, // 担保品转出
  Unknown = 99
};

NLOHMANN_JSON_SERIALIZE_ENUM(Side, {
                                       {Side::Buy, "Buy"},
                                       {Side::Sell, "Sell"},
                                       {Side::Lock, "Lock"},
                                       {Side::Unlock, "Unlock"},
                                       {Side::Exec, "Exec"},
                                       {Side::Drop, "Drop"},
                                       {Side::Purchase, "Purchase"},
                                       {Side::Redemption, "Redemption"},
                                       {Side::Split, "Split"},
                                       {Side::Merge, "Merge"},
                                       {Side::MarginTrade, "MarginTrade"},
                                       {Side::ShortSell, "ShortSell"},
                                       {Side::RepayMargin, "RepayMargin"},
                                       {Side::RepayStock, "RepayStock"},
                                       {Side::CashRepayMargin, "CashRepayMargin"},
                                       {Side::StockRepayStock, "StockRepayStock"},
                                       {Side::SurplusStockTransfer, "SurplusStockTransfer"},
                                       {Side::GuaranteeStockTransferIn, "GuaranteeStockTransferIn"},
                                       {Side::GuaranteeStockTransferOut, "GuaranteeStockTransferOut"},
                                       {Side::Unknown, "Unknown"},
                                   })

inline std::ostream &operator<<(std::ostream &os, Side t) { return os << int8_t(t); }

enum class Offset : int8_t { Open, Close, CloseToday, CloseYesterday };

NLOHMANN_JSON_SERIALIZE_ENUM(Offset, {
                                         {Offset::Open, "Open"},
                                         {Offset::Close, "Close"},
                                         {Offset::CloseToday, "CloseToday"},
                                         {Offset::CloseYesterday, "CloseYesterday"},
                                     })

inline std::ostream &operator<<(std::ostream &os, Offset t) { return os << int8_t(t); }

enum class HedgeFlag : int8_t { Speculation, Arbitrage, Hedge, Covered };

NLOHMANN_JSON_SERIALIZE_ENUM(HedgeFlag, {
                                            {HedgeFlag::Speculation, "Speculation"},
                                            {HedgeFlag::Arbitrage, "Arbitrage"},
                                            {HedgeFlag::Hedge, "Hedge"},
                                            {HedgeFlag::Covered, "Covered"},
                                        })

inline std::ostream &operator<<(std::ostream &os, HedgeFlag t) { return os << int8_t(t); }

enum class OrderActionFlag : int8_t {
  Cancel,
};

NLOHMANN_JSON_SERIALIZE_ENUM(OrderActionFlag, {
                                                  {OrderActionFlag::Cancel, "Cancel"},
                                              })

inline std::ostream &operator<<(std::ostream &os, OrderActionFlag t) { return os << int8_t(t); }

enum class PriceType : int8_t {
  Limit, // 限价,证券通用
  Any, // 市价，证券通用，对于股票上海为最优五档剩余撤销，深圳为即时成交剩余撤销，建议客户采用
  FakBest5,    // 上海深圳最优五档即时成交剩余撤销，不需要报价
  ForwardBest, // 深圳本方方最优价格申报, 不需要报价
  ReverseBest, // 上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报，不需要报价
  Fak,         // 深圳即时成交剩余撤销，不需要报价
  Fok,         // 深圳市价全额成交或者撤销，不需要报价
  Unknown
};

NLOHMANN_JSON_SERIALIZE_ENUM(PriceType, {
                                            {PriceType::Limit, "Limit"},
                                            {PriceType::Any, "Any"},
                                            {PriceType::FakBest5, "FakBest5"},
                                            {PriceType::ForwardBest, "ForwardBest"},
                                            {PriceType::ReverseBest, "ReverseBest"},
                                            {PriceType::Fak, "Fak"},
                                            {PriceType::Fok, "Fok"},
                                            {PriceType::Unknown, "Unknown"},
                                        })

inline std::ostream &operator<<(std::ostream &os, PriceType t) { return os << int8_t(t); }

enum class PriceLevel : int8_t {
  Lastest, // 最新价
  Sell5,
  Sell4,
  Sell3,
  Sell2,
  Sell1,
  Buy1,
  Buy2,
  Buy3,
  Buy4,
  Buy5,
  UpperLimitPrice, // 涨停价
  LowerLimitPrice, // 跌停价
  Unknown
};

NLOHMANN_JSON_SERIALIZE_ENUM(PriceLevel, {
                                             {PriceLevel::Lastest, "Lastest"},
                                             {PriceLevel::Sell5, "Sell5"},
                                             {PriceLevel::Sell4, "Sell4"},
                                             {PriceLevel::Sell3, "Sell3"},
                                             {PriceLevel::Sell2, "Sell2"},
                                             {PriceLevel::Sell1, "Sell1"},
                                             {PriceLevel::Buy5, "Buy5"},
                                             {PriceLevel::Buy4, "Buy4"},
                                             {PriceLevel::Buy3, "Buy3"},
                                             {PriceLevel::Buy2, "Buy2"},
                                             {PriceLevel::Buy1, "Buy1"},
                                             {PriceLevel::UpperLimitPrice, "UpperLimitPrice"},
                                             {PriceLevel::LowerLimitPrice, "LowerLimitPrice"},
                                             {PriceLevel::Unknown, "Unknown"},
                                         })

inline std::ostream &operator<<(std::ostream &os, PriceLevel t) { return os << int8_t(t); }

enum class VolumeCondition : int8_t { Any, Min, All };

NLOHMANN_JSON_SERIALIZE_ENUM(VolumeCondition, {
                                                  {VolumeCondition::Any, "Any"},
                                                  {VolumeCondition::Min, "Min"},
                                                  {VolumeCondition::All, "All"},
                                              })

inline std::ostream &operator<<(std::ostream &os, VolumeCondition t) { return os << int8_t(t); }

enum class TimeCondition : int8_t { IOC, GFD, GTC };

NLOHMANN_JSON_SERIALIZE_ENUM(TimeCondition, {
                                                {TimeCondition::IOC, "IOC"},
                                                {TimeCondition::GFD, "GFD"},
                                                {TimeCondition::GTC, "GTC"},
                                            })

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

NLOHMANN_JSON_SERIALIZE_ENUM(OrderStatus, {
                                              {OrderStatus::Unknown, "Unknown"},
                                              {OrderStatus::Submitted, "Submitted"},
                                              {OrderStatus::Pending, "Pending"},
                                              {OrderStatus::Cancelled, "Cancelled"},
                                              {OrderStatus::Error, "Error"},
                                              {OrderStatus::Filled, "Filled"},
                                              {OrderStatus::PartialFilledNotActive, "PartialFilledNotActive"},
                                              {OrderStatus::PartialFilledActive, "PartialFilledActive"},
                                              {OrderStatus::Lost, "Lost"},
                                          })

inline std::ostream &operator<<(std::ostream &os, OrderStatus t) { return os << int8_t(t); }

enum class BasketOrderStatus : int8_t { Unknown, Pending, PartialFilledNotActive, PartialFilledActive, Filled };

NLOHMANN_JSON_SERIALIZE_ENUM(BasketOrderStatus,
                             {
                                 {BasketOrderStatus::Unknown, "Unknown"},
                                 {BasketOrderStatus::Pending, "Pending"},
                                 {BasketOrderStatus::PartialFilledNotActive, "PartialFilledNotActive"},
                                 {BasketOrderStatus::PartialFilledActive, "PartialFilledActive"},
                                 {BasketOrderStatus::Filled, "Filled"},
                             })

inline std::ostream &operator<<(std::ostream &os, BasketOrderStatus t) { return os << int8_t(t); }

enum class BasketOrderCalculationMode : int8_t { Static, Dynamic };

NLOHMANN_JSON_SERIALIZE_ENUM(BasketOrderCalculationMode, {
                                                             {BasketOrderCalculationMode::Static, "Static"},
                                                             {BasketOrderCalculationMode::Dynamic, "Dynamic"},
                                                         })

inline std::ostream &operator<<(std::ostream &os, BasketOrderCalculationMode t) { return os << int8_t(t); }

enum class BasketVolumeType : int8_t { Unknown, Quantity, Proportion };

NLOHMANN_JSON_SERIALIZE_ENUM(BasketVolumeType, {
                                                   {BasketVolumeType::Unknown, "Unknown"},
                                                   {BasketVolumeType::Quantity, "Quantity"},
                                                   {BasketVolumeType::Proportion, "Proportion"},
                                               })

inline std::ostream &operator<<(std::ostream &os, BasketVolumeType t) { return os << int8_t(t); }

enum class BasketType : int8_t { Custom, ETF };

NLOHMANN_JSON_SERIALIZE_ENUM(BasketType, {
                                             {BasketType::Custom, "Custom"},
                                             {BasketType::ETF, "ETF"},
                                         })

inline std::ostream &operator<<(std::ostream &os, BasketType t) { return os << int8_t(t); }

enum class Direction : int8_t { Long, Short };

NLOHMANN_JSON_SERIALIZE_ENUM(Direction, {
                                            {Direction::Long, "Long"},
                                            {Direction::Short, "Short"},
                                        })

inline std::ostream &operator<<(std::ostream &os, Direction t) { return os << int8_t(t); }

enum class AccountType : int8_t { Stock, Credit, Future };

NLOHMANN_JSON_SERIALIZE_ENUM(AccountType, {
                                              {AccountType::Stock, "Stock"},
                                              {AccountType::Credit, "Credit"},
                                              {AccountType::Future, "Future"},
                                          })

inline std::ostream &operator<<(std::ostream &os, AccountType t) { return os << int8_t(t); }

enum class CommissionRateMode : int8_t { ByAmount, ByVolume };

NLOHMANN_JSON_SERIALIZE_ENUM(CommissionRateMode, {
                                                     {CommissionRateMode::ByAmount, "ByAmount"},
                                                     {CommissionRateMode::ByVolume, "ByVolume"},
                                                 })

inline std::ostream &operator<<(std::ostream &os, CommissionRateMode t) { return os << int8_t(t); }

enum class LedgerCategory : int8_t {
  Account,
  Strategy,
};

NLOHMANN_JSON_SERIALIZE_ENUM(LedgerCategory, {
                                                 {LedgerCategory::Account, "Account"},
                                                 {LedgerCategory::Strategy, "Strategy"},
                                             })

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

NLOHMANN_JSON_SERIALIZE_ENUM(BrokerState, {
                                              {BrokerState::Pending, "Pending"},
                                              {BrokerState::Idle, "Idle"},
                                              {BrokerState::DisConnected, "DisConnected"},
                                              {BrokerState::Connected, "Connected"},
                                              {BrokerState::LoggedIn, "LoggedIn"},
                                              {BrokerState::LoginFailed, "LoginFailed"},
                                              {BrokerState::Ready, "Ready"},
                                          })

inline std::ostream &operator<<(std::ostream &os, BrokerState t) { return os << int8_t(t); }

enum class HistoryDataType : int8_t { Normal = 0, PageEnd = 1, TotalEnd = 2 };

NLOHMANN_JSON_SERIALIZE_ENUM(HistoryDataType, {
                                                  {HistoryDataType::Normal, "Normal"},
                                                  {HistoryDataType::PageEnd, "PageEnd"},
                                                  {HistoryDataType::TotalEnd, "TotalEnd"},
                                              })

inline std::ostream &operator<<(std::ostream &os, HistoryDataType t) { return os << int8_t(t); }

enum class StrategyState : int8_t { Normal, Warn, Error };

NLOHMANN_JSON_SERIALIZE_ENUM(StrategyState, {
                                                {StrategyState::Normal, "Normal"},
                                                {StrategyState::Warn, "Warn"},
                                                {StrategyState::Error, "Error"},
                                            })

inline std::ostream &operator<<(std::ostream &os, StrategyState t) { return os << int8_t(t); }

class AssembleMode {
public:
  inline static const uint32_t Channel = 0b00000001; // read only journal of location to dest_id
  inline static const uint32_t Write = 0b00000010;   // read all journal from this location
  inline static const uint32_t Read = 0b00000100;    // read all journal to this dest_id
  inline static const uint32_t Public = 0b00001000;  // read all journal to location::PUBLIC
  inline static const uint32_t All = 0b00010000;     // read all journal
};

template <typename T, typename U> inline T sub_data_bitwise(const T &a, const T &b) {
  return static_cast<T>(static_cast<U>(a) | static_cast<U>(b));
}
} // namespace kungfu::longfist::enums
#endif // KUNGFU_LONGFIST_ENUM_H
