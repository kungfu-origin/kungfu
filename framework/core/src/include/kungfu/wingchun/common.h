// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>
#include <locale>

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/util.h>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define EXCHANGE_SSE "SSE"
#define EXCHANGE_SZE "SZE"
#define EXCHANGE_BSE "BSE"
#define EXCHANGE_SHFE "SHFE"
#define EXCHANGE_DCE "DCE"
#define EXCHANGE_CZCE "CZCE"
#define EXCHANGE_CFFEX "CFFEX"
#define EXCHANGE_INE "INE"
#define EXCHANGE_BINANCE "BINANCE"
#define EXCHANGE_HB "HB"

// 全市场exchange id定义
#define EXCHANGE_HK "HK"           // 港股: 4（香港交易所）
#define EXCHANGE_HK_FUTURE "HKFUT" // 港股期货: 5（香港交易所）
#define EXCHANGE_US "US"           // 美股: 29（纳斯达克交易所）
#define EXCHANGE_US_FUTURE "USFUT" // 美股: 29（纳斯达克交易所）
#define EXCHANGE_GLFX "GLFX"       // 全球外汇: 41
#define EXCHANGE_IPE "IPE"         // LME\IPE: 45(LME（伦敦金属交易所）、ICE)
#define EXCHANGE_CBOT "CBOT"       // ES-CBOT: 62
#define EXCHANGE_CEC "CEC"         // ES-CEC: 63
#define EXCHANGE_LIFE "LIFE"       // ES-LIFE: 64
#define EXCHANGE_MTIF "MTIF"       // ES-MTIF: 65
#define EXCHANGE_NYCE "NYCE"       // ES-NYCE: 66、49
#define EXCHANGE_CMX "CMX"         // ES-CMX: 67
#define EXCHANGE_NYME "NYME"       // ES-NYME: 68
#define EXCHANGE_SIME "SIME"       // ES-SIME: 69
#define EXCHANGE_CME "CME"         // ES-CME: 70
#define EXCHANGE_IMM "IMM"         // ES-IMM: 71
#define EXCHANGE_WIDX "WIDX"       // ES-WIDX: 72
#define EXCHANGE_FREX "FREX"       // ES-FREX: 73
#define EXCHANGE_METL "METL"       // ES-METL: 74
#define EXCHANGE_IPM "IPM"         // 国际贵金属: 5000

#define SOURCE_SIM "sim"
#define SOURCE_CTP "ctp"
#define SOURCE_XTP "xtp"
#define SOURCE_BAC "barrich"

#define EPSILON (1e-6)
#define DOUBLEMAX (1e16) // 一亿亿, 2018年A股总市值不到50万亿

namespace kungfu::wingchun {
constexpr int64_t VOLUME_ZERO = 0;

class wingchun_error : public std::runtime_error {
public:
  explicit wingchun_error(const std::string &_s) : std::runtime_error(_s) {}

  [[maybe_unused]] explicit wingchun_error(const char *_s) : std::runtime_error(_s) {}

  ~wingchun_error() noexcept override = default;
};

inline bool is_greater(double x, double y) { return std::isgreater(x - y, EPSILON); }

inline bool is_less(double x, double y) { return std::isless(x - y, EPSILON * -1); }

inline bool is_equal(double x, double y) { return std::abs(x - y) <= EPSILON * std::abs(x); }

[[maybe_unused]] inline bool is_greater_equal(double x, double y) { return is_greater(x, y) || is_equal(x, y); }

inline bool is_less_equal(double x, double y) { return is_less(x, y) || is_equal(x, y); }

inline bool is_zero(double x) { return is_equal(x, 0.0); }

inline bool is_too_large(double x) { return is_greater(x, DOUBLEMAX); }

inline bool is_valid_price(double price) { return !is_less_equal(price, 0.0) && !is_too_large(price); }

[[maybe_unused]] inline double rounded(double x, int n) {
  if (is_too_large(x) || is_zero(x) || is_too_large(std::abs(x))) {
    return 0.0;
  } else {
    char out[64];
    double xrounded;
    sprintf(out, "%.*f", n, x);
    xrounded = strtod(out, nullptr);
    return xrounded;
  }
}

inline bool string_equals(const std::string &s1, const std::string &s2) {
  return std::strcmp(s1.c_str(), s2.c_str()) == 0;
}

inline bool string_equals_n(const std::string &s1, const std::string &s2, size_t l) {
  return std::strncmp(s1.c_str(), s2.c_str(), l) == 0;
}

[[maybe_unused]] inline bool endswith(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

inline bool startswith(const std::string &str, const std::string &prefix) {
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

inline bool is_final_status(const longfist::enums::OrderStatus &status) {
  switch (status) {
  case longfist::enums::OrderStatus::Submitted:
  case longfist::enums::OrderStatus::Pending:
  case longfist::enums::OrderStatus::PartialFilledActive:
  case longfist::enums::OrderStatus::Unknown:
    return false;
  default:
    return true;
  }
}

inline bool is_final_basket_order_status(const longfist::enums::BasketOrderStatus &status) {
  switch (status) {
  case longfist::enums::BasketOrderStatus::Unknown:
  case longfist::enums::BasketOrderStatus::Pending:
    return false;
  default:
    return true;
  }
}

inline bool is_convertible_bond(const std::string &instrument_id, const std::string &exchange_id) {
  return ((string_equals_n(instrument_id, "123", 3) || string_equals_n(instrument_id, "128", 3) ||
           string_equals_n(instrument_id, "117", 3) || string_equals_n(instrument_id, "127", 3) ||
           string_equals_n(instrument_id, "125", 3) || string_equals_n(instrument_id, "126", 3)) &&
          string_equals(exchange_id, EXCHANGE_SZE)) ||
         ((string_equals_n(instrument_id, "110", 3) || string_equals_n(instrument_id, "113", 3)) &&
          string_equals(exchange_id, EXCHANGE_SSE));
}

inline bool is_repo(const std::string &instrument_id, const std::string &exchange_id) {
  return (string_equals_n(instrument_id, "204", 3) && string_equals(exchange_id, EXCHANGE_SSE)) ||
         (string_equals_n(instrument_id, "1318", 4) && string_equals(exchange_id, EXCHANGE_SZE));
}

inline double get_repo_commission_rate(const std::string &instrument_id) {
  if (string_equals(instrument_id, "204001") || string_equals(instrument_id, "131810")) {
    return 0.00001;
  } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811")) {
    return 0.00002;
  } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800")) {
    return 0.00003;
  } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809")) {
    return 0.00004;
  } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801")) {
    return 0.00005;
  } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802")) {
    return 0.00010;
  } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803")) {
    return 0.00020;
  } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805")) {
    return 0.00030;
  } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806")) {
    return 0.00030;
  } else {
    return 0.0;
  }
}

inline int get_repo_expire_days(const std::string &instrument_id) {
  if (string_equals(instrument_id, "204001") || string_equals(instrument_id, "131810")) {
    return 1;
  } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811")) {
    return 2;
  } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800")) {
    return 3;
  } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809")) {
    return 4;
  } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801")) {
    return 7;
  } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802")) {
    return 14;
  } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803")) {
    return 28;
  } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805")) {
    return 91;
  } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806")) {
    return 182;
  } else {
    return 0;
  }
}

// 标的代码和类型
struct HKCodeRule {
  int beg = 0;
  int end = 0;
  longfist::enums::InstrumentType type;
};

// 获取港股标的类型
inline longfist::enums::InstrumentType get_instrument_type_by_exchange_hk(const std::string &instrument_id) {
  // 查看地址：https://www.hkex.com.hk/-/media/HKEX-Market/Products/Securities/Stock-Code-Allocation-Plan/scap_c.pdf
  static std::vector<HKCodeRule> hk_code_type_def = {
      {1, 2799, longfist::enums::InstrumentType::Stock},
      {2800, 2849, longfist::enums::InstrumentType::Fund}, // 交易所買賣基金
      {3000, 3199, longfist::enums::InstrumentType::Fund},
      {3400, 3499, longfist::enums::InstrumentType::Fund},
      {4000, 4199, longfist::enums::InstrumentType::Bond}, // 香港金融管理局的外匯基金債券
      {4200, 4299, longfist::enums::InstrumentType::Bond},
      {4300, 4329, longfist::enums::InstrumentType::Bond},
      {4400, 4599, longfist::enums::InstrumentType::Bond},
      {5000, 6029, longfist::enums::InstrumentType::Bond},
      {40000, 40999, longfist::enums::InstrumentType::Bond},
      {4600, 4699, longfist::enums::InstrumentType::Stock},
      {4700, 4799, longfist::enums::InstrumentType::Bond},
      {4800, 4999, longfist::enums::InstrumentType::Stock}, // SPAC 權證
      {6200, 6299, longfist::enums::InstrumentType::Stock},
      {6300, 6399, longfist::enums::InstrumentType::Stock},
      {6750, 6799, longfist::enums::InstrumentType::Bond},  // 中華人民共和國財政部債券
      {7200, 7399, longfist::enums::InstrumentType::Stock}, // 槓桿及反向產品
      {7500, 7599, longfist::enums::InstrumentType::Stock},
      {7800, 7999, longfist::enums::InstrumentType::Stock}, // SPAC 股份
      {8000, 8999, longfist::enums::InstrumentType::Stock},
      {9000, 9199, longfist::enums::InstrumentType::Fund}, // 交易所買賣基金(以美元買賣)
      {9400, 9499, longfist::enums::InstrumentType::Fund},
      {9800, 9849, longfist::enums::InstrumentType::Fund},
      {9200, 9399, longfist::enums::InstrumentType::Stock}, // 槓桿及反向產品(以美元
      {9500, 9599, longfist::enums::InstrumentType::Stock},
      {10000, 29999, longfist::enums::InstrumentType::StockOption}, // 衍生權證
      {30000, 39999, longfist::enums::InstrumentType::Stock},       // 供日後使用
      {41000, 46999, longfist::enums::InstrumentType::Stock},       // 供日後使用
      {47000, 48999, longfist::enums::InstrumentType::Warrant},     // 界內證
      {49000, 49999, longfist::enums::InstrumentType::Stock},       // 供日後使用
      {50000, 69999, longfist::enums::InstrumentType::Warrant},
      {70000, 79999, longfist::enums::InstrumentType::Stock}, // 供日後使用
      {82800, 82849, longfist::enums::InstrumentType::Fund},  // 交易所買賣基金
      {83000, 83199, longfist::enums::InstrumentType::Fund},
      {83400, 83499, longfist::enums::InstrumentType::Fund},
      {84300, 84329, longfist::enums::InstrumentType::Bond}, // 僅售予專業投資者的債務證券
      {84400, 84599, longfist::enums::InstrumentType::Bond},
      {85000, 85743, longfist::enums::InstrumentType::Bond},
      {85901, 86029, longfist::enums::InstrumentType::Bond},
      {84600, 84699, longfist::enums::InstrumentType::Stock}, // 僅售予專業投資者優先股
      {85744, 85900, longfist::enums::InstrumentType::Stock},
      {86600, 86799, longfist::enums::InstrumentType::Bond}, // 中華人民共和國財政部債券
      {87000, 87099,
       longfist::enums::InstrumentType::Fund}, // 房地產投資信託基金及交易所買賣基金以外的單位信託 / 互惠基金
      {87200, 87399, longfist::enums::InstrumentType::Stock},       // 槓桿及反向產品
      {87500, 87599, longfist::enums::InstrumentType::Stock},       // 槓桿及反向產品
      {89000, 89999, longfist::enums::InstrumentType::StockOption}, // 衍生權證
      {90000, 99999, longfist::enums::InstrumentType::Stock},       // 供日後使用
  };

  int nId = atoi(instrument_id.c_str());

  for (auto &iter : hk_code_type_def) {
    if (nId >= iter.beg && nId <= iter.end) {
      return iter.type;
    }
  }

  return longfist::enums::InstrumentType::Stock;
}

inline longfist::enums::InstrumentType get_instrument_type(const std::string &exchange_id,
                                                           const std::string &instrument_id) {
  if (string_equals(exchange_id, EXCHANGE_SSE)) {
    if (instrument_id.length() == 8) {
      return longfist::enums::InstrumentType::StockOption;
    }
    if (startswith(instrument_id, "00")) {
      return longfist::enums::InstrumentType::Index;
    } else if (startswith(instrument_id, "0")) {
      return longfist::enums::InstrumentType::Bond;
    } else if (startswith(instrument_id, "1")) {
      return longfist::enums::InstrumentType::Bond;
    } else if (startswith(instrument_id, "2")) {
      return longfist::enums::InstrumentType::Repo;
    } else if (startswith(instrument_id, "5")) {
      return longfist::enums::InstrumentType::Fund;
    } else if (startswith(instrument_id, "688") || startswith(instrument_id, "787") ||
               startswith(instrument_id, "789")) {
      return longfist::enums::InstrumentType::TechStock;
    }

    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_SZE)) {
    if (instrument_id.length() == 8) {
      return longfist::enums::InstrumentType::StockOption;
    }
    if (startswith(instrument_id, "15") || startswith(instrument_id, "16") || startswith(instrument_id, "18")) {
      return longfist::enums::InstrumentType::Fund;
    } else if (startswith(instrument_id, "131")) {
      return longfist::enums::InstrumentType::Repo;
    } else if (startswith(instrument_id, "1")) {
      return longfist::enums::InstrumentType::Bond;
    }
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_BSE)) {
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_DCE) || string_equals(exchange_id, EXCHANGE_SHFE) ||
             string_equals(exchange_id, EXCHANGE_CFFEX) || string_equals(exchange_id, EXCHANGE_CZCE) ||
             string_equals(exchange_id, EXCHANGE_INE)) {
    return longfist::enums::InstrumentType::Future;
  } else if (string_equals(exchange_id, EXCHANGE_BINANCE) || string_equals(exchange_id, EXCHANGE_HB)) {
    return longfist::enums::InstrumentType::Crypto;
  } else if (string_equals(exchange_id, EXCHANGE_HK)) {
    return get_instrument_type_by_exchange_hk(instrument_id);
  } else if (string_equals(exchange_id, EXCHANGE_HK_FUTURE)) {
    return longfist::enums::InstrumentType::Future;
  } else if (string_equals(exchange_id, EXCHANGE_US)) {
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_US_FUTURE)) {
    return longfist::enums::InstrumentType::Future;
  }
  SPDLOG_ERROR("invalid instrument type for exchange {} and instrument {}", exchange_id, instrument_id);
  return longfist::enums::InstrumentType::Unknown;
}

inline std::string str_from_instrument_type(longfist::enums::InstrumentType type) {
  switch (type) {
  case longfist::enums::InstrumentType::Unknown:
    return "Unknown";
  case longfist::enums::InstrumentType::Stock:
    return "Stock";
  case longfist::enums::InstrumentType::Future:
    return "Future";
  case longfist::enums::InstrumentType::Bond:
    return "Bond";
  case longfist::enums::InstrumentType::StockOption:
    return "StockOption";
  case longfist::enums::InstrumentType::TechStock:
    return "TechStock";
  case longfist::enums::InstrumentType::Fund:
    return "Fund";
  case longfist::enums::InstrumentType::Index:
    return "Index";
  case longfist::enums::InstrumentType::Repo:
    return "Repo";
  case longfist::enums::InstrumentType::Warrant:
    return "Warrant";
  case longfist::enums::InstrumentType::Iopt:
    return "Iopt";
  case longfist::enums::InstrumentType::Crypto:
    return "Crypto";
  default:
    return "Unknown";
  }
}

inline longfist::enums::SubscribeInstrumentType
instrument_type_to_subscribe_instrument_type(longfist::enums::InstrumentType instrument_type) {
  switch (instrument_type) {
  case longfist::enums::InstrumentType::Stock:
    return longfist::enums::SubscribeInstrumentType::Stock;
  case longfist::enums::InstrumentType::Future:
    return longfist::enums::SubscribeInstrumentType::Future;
  case longfist::enums::InstrumentType::Bond:
    return longfist::enums::SubscribeInstrumentType::Bond;
  case longfist::enums::InstrumentType::Repo:
    return longfist::enums::SubscribeInstrumentType::Bond;
  case longfist::enums::InstrumentType::StockOption:
    return longfist::enums::SubscribeInstrumentType::StockOption;
  case longfist::enums::InstrumentType::TechStock:
    return longfist::enums::SubscribeInstrumentType::Stock;
  case longfist::enums::InstrumentType::Fund:
    return longfist::enums::SubscribeInstrumentType::Fund;
  case longfist::enums::InstrumentType::Index:
    return longfist::enums::SubscribeInstrumentType::Index;
  default:
    return longfist::enums::SubscribeInstrumentType::All;
  }
}

inline std::string get_instrument_product(const char *instrument_id) {
  std::string product = {};
  int i = 0;
  while (instrument_id[i] != 0) {
    if (instrument_id[i] < '0' || instrument_id[i] > '9') {
      product.push_back(std::toupper(instrument_id[i]));
    }
    ++i;
  }
  return product;
}

inline longfist::enums::Direction get_future_direction(longfist::enums::Side side, longfist::enums::Offset offset) {
  if ((side == longfist::enums::Side::Buy && offset == longfist::enums::Offset::Open) ||
      (side == longfist::enums::Side::Sell && offset != longfist::enums::Offset::Open)) {
    return longfist::enums::Direction::Long;
  } else {
    return longfist::enums::Direction::Short;
  }
}

inline bool is_open(longfist::enums::InstrumentType instrument_type, longfist::enums::Side side,
                    longfist::enums::Offset offset) {
  if (instrument_type == longfist::enums::InstrumentType::Stock ||
      instrument_type == longfist::enums::InstrumentType::Bond) {
    return side == longfist::enums::Side::Buy;
  } else if (instrument_type == longfist::enums::InstrumentType::Future) {
    return offset == longfist::enums::Offset::Open;
  } else {
    return false;
  }
}

inline std::string get_exchange_id_from_future_instrument_id(const std::string &instrument_id) {
  std::size_t found = instrument_id.find_first_of("0123456789");
  std::string product = instrument_id.substr(0, found);
  std::transform(product.begin(), product.end(), product.begin(), ::tolower);
  if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
      product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "rr" || product == "l" ||
      product == "v" || product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg" ||
      product == "eb") {
    return EXCHANGE_DCE;
  } else if (product == "wh" || product == "pm" || product == "cf" || product == "sr" || product == "oi" ||
             product == "ri" || product == "rs" || product == "rm" || product == "jr" || product == "lr" ||
             product == "cy" || product == "ap" || product == "cj" || product == "ta" || product == "ma" ||
             product == "zc" || product == "sf" || product == "sm" || product == "ur" || product == "fg" ||
             product == "sa") {
    return EXCHANGE_CZCE;
  } else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" ||
             product == "sn" || product == "au" || product == "ag" || product == "rb" || product == "wr" ||
             product == "hc" || product == "ss" || product == "fu" || product == "bu" || product == "ru" ||
             product == "nr" || product == "sp") {
    return EXCHANGE_SHFE;
  } else if (product == "if" || product == "ic" || product == "ih" || product == "ts" || product == "tf" ||
             product == "t") {
    return EXCHANGE_CFFEX;
  } else if (product == "sc") {
    return EXCHANGE_INE;
  } else {
    return "";
  }
}

inline bool is_shortable(longfist::enums::InstrumentType instrument_type) {
  using namespace longfist::enums;
  return not(instrument_type == InstrumentType::Stock or instrument_type == InstrumentType::Bond or
             instrument_type == InstrumentType::Fund or instrument_type == InstrumentType::TechStock or
             instrument_type == InstrumentType::Index or instrument_type == InstrumentType::Repo);
}

inline longfist::enums::Direction get_direction(longfist::enums::InstrumentType instrument_type,
                                                longfist::enums::Side side, longfist::enums::Offset offset) {
  using namespace longfist::enums;

  if (side == Side::MarginTrade) {
    return Direction::Long;
  } else if (side == Side::ShortSell) {
    return Direction::Short;
  } else if (side == Side::RepayMargin) {
    return Direction::Long;
  } else if (side == Side::RepayStock) {
    return Direction::Short;
  }

  if (side == Side::Exec) {
    return Direction::Long;
  }

  if (not is_shortable(instrument_type)) {
    return Direction::Long;
  }
  if (side == Side::Buy and offset == Offset::Open) {
    return Direction::Long;
  }
  if (side == Side::Sell and
      (offset == Offset::Close or offset == Offset::CloseToday or offset == Offset::CloseYesterday)) {
    return Direction::Long;
  }
  if (side == Side::Sell and offset == Offset::Open) {
    return Direction::Short;
  }
  if (side == Side::Buy and
      (offset == Offset::Close or offset == Offset::CloseToday or offset == Offset::CloseYesterday)) {
    return Direction::Short;
  }
  throw wingchun_error(fmt::format("get_direction error: invalid direction args {} {} {}", (int)instrument_type,
                                   (int)side, (int)offset));
}

inline uint32_t hash_instrument(const char *exchange_id, const char *instrument_id) {
  return yijinjing::util::hash_str_32(instrument_id) ^ yijinjing::util::hash_str_32(exchange_id);
}

inline uint32_t hash_basket_instrument(uint32_t basket_uid, const char *exchange_id, const char *instrument_id) {
  return basket_uid ^ yijinjing::util::hash_str_32(instrument_id) ^ yijinjing::util::hash_str_32(exchange_id);
}

inline uint32_t hash_account(const std::string &source_name, const std::string &account_id) {
  return yijinjing::util::hash_str_32(source_name) ^ yijinjing::util::hash_str_32(account_id);
}

inline void order_from_input(const longfist::types::OrderInput &input, longfist::types::Order &order) {
  order.order_id = input.order_id;

  strcpy(order.instrument_id, input.instrument_id);
  strcpy(order.exchange_id, input.exchange_id);

  order.instrument_type = input.instrument_type;

  order.limit_price = input.limit_price;
  order.frozen_price = input.frozen_price;

  order.volume = input.volume;
  order.volume_left = input.volume;
  order.status = longfist::enums::OrderStatus::Pending;

  order.side = input.side;
  order.offset = input.offset;

  order.is_swap = input.is_swap;

  order.hedge_flag = input.hedge_flag;
  order.price_type = input.price_type;
  order.volume_condition = input.volume_condition;
  order.time_condition = input.time_condition;

  order.parent_id = input.parent_id;
}

} // namespace kungfu::wingchun

#endif // WINGCHUN_COMMON_H
