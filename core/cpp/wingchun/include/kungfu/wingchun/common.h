//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>
#include <string>
#include <fmt/format.h>
#include <kungfu/yijinjing/util/util.h>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define EXCHANGE_SSE "SSE"
#define EXCHANGE_SZE "SZE"
#define EXCHANGE_SHFE "SHFE"
#define EXCHANGE_DCE "DCE"
#define EXCHANGE_CZCE "CZCE"
#define EXCHANGE_CFFEX "CFFEX"
#define EXCHANGE_INE "INE"

#define SOURCE_SIM "sim"
#define SOURCE_CTP "ctp"
#define SOURCE_XTP "xtp"

#define EPSILON (1e-6)
#define DOUBLEMAX (1e16) // 一亿亿, 2018年A股总市值不到50万亿

namespace kungfu
{
    namespace wingchun
    {
        const int INSTRUMENT_ID_LEN = 32;
        const int PRODUCT_ID_LEN = 32;
        const int DATE_LEN = 9;
        const int EXCHANGE_ID_LEN = 16;
        const int ACCOUNT_ID_LEN = 32;
        const int CLIENT_ID_LEN = 32;
        const int EXEC_ID_LEN = 32;
        const int SOURCE_ID_LEN = 16;
        const int BROKER_ID_LEN = 32;
        const int ERROR_MSG_LEN = 128;

        enum class InstrumentType: int8_t
        {
            Unknown,       //未知
            Stock,         //普通股票
            Future,        //期货
            Bond,          //债券
            StockOption,   //股票期权
            Fund,          //基金
            TechStock,     //科创板股票
            Index,         //指数
            Repo           //回购
        };

        enum class ExecType: int8_t
        {
            Unknown,
            Cancel,
            Trade
        };

        enum class BsFlag: int8_t
        {
            Unknown,
            Buy,
            Sell
        };

        enum class Side: int8_t
        {
            Buy,
            Sell,
            Lock,
            Unlock,
            Exec,
            Drop
        };

        enum class Offset: int8_t
        {
            Open,
            Close,
            CloseToday,
            CloseYesterday
        };

        enum class HedgeFlag: int8_t
        {
            Speculation,
            Arbitrage,
            Hedge,
            Covered
        };

        enum class OrderActionFlag: int8_t
        {
            Cancel,
        };

        enum class PriceType: int8_t
        {
            Limit,                  //限价,证券通用
            Any,                    //市价，证券通用，对于股票上海为最优五档剩余撤销，深圳为即时成交剩余撤销，建议客户采用
            FakBest5,               //上海深圳最优五档即时成交剩余撤销，不需要报价
            ForwardBest,            //深圳本方方最优价格申报, 不需要报价
            ReverseBest,            //上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报，不需要报价
            Fak,                    //深圳即时成交剩余撤销，不需要报价
            Fok,                    //深圳市价全额成交或者撤销，不需要报价
            UnKnown
        };

        enum class VolumeCondition: int8_t
        {
            Any,
            Min,
            All
        };

        enum class TimeCondition: int8_t
        {
            IOC,
            GFD,
            GTC
        };

        enum class OrderStatus: int8_t
        {
            Unknown,
            Submitted,
            Pending,
            Cancelled,
            Error,
            Filled,
            PartialFilledNotActive,
            PartialFilledActive
        };

        enum class Direction: int8_t
        {
            Long,
            Short
        };

        enum class AccountType: int8_t
        {
            Stock,
            Credit,
            Future
        };

        enum class CommissionRateMode: int8_t
        {
            ByAmount,
            ByVolume
        };

        enum class LedgerCategory: int8_t
        {
            Account,
            Strategy,
        };

        class wingchun_error : public std::runtime_error
        {
        public:
            explicit wingchun_error(const std::string &__s) : std::runtime_error(__s)
            {}

            explicit wingchun_error(const char *__s) : std::runtime_error(__s)
            {}

            virtual ~wingchun_error() noexcept = default;
        };

        inline bool is_greater(double x, double y)
        {
            return std::isgreater(x - y, EPSILON);
        }

        inline bool is_less(double x, double y)
        {
            return std::isless(x - y, EPSILON * -1);
        }

        inline bool is_equal(double x, double y)
        {
            return std::abs(x - y) <= EPSILON * std::abs(x);
        }

        inline bool is_greater_equal(double x, double y)
        {
            return is_greater(x, y) || is_equal(x, y);
        }

        inline bool is_less_equal(double x, double y)
        {
            return is_less(x, y) || is_equal(x, y);
        }

        inline bool is_zero(double x)
        {
            return is_equal(x, 0.0);
        }

        inline bool is_too_large(double x)
        {
            return is_greater(x, DOUBLEMAX);
        }

        inline bool is_valid_price(double price)
        {
            return !is_less_equal(price, 0.0) && !is_too_large(price);
        }

        inline double rounded(double x, int n)
        {
            if (is_too_large(x) || is_zero(x) || is_too_large(std::abs(x)))
            {
                return 0.0;
            } else
            {
                char out[64];
                double xrounded;
                sprintf(out, "%.*f", n, x);
                xrounded = strtod(out, 0);
                return xrounded;
            }
        }

        inline bool string_equals(const std::string &s1, const std::string &s2)
        {
            return std::strcmp(s1.c_str(), s2.c_str()) == 0;
        }

        inline bool string_equals_n(const std::string &s1, const std::string &s2, size_t l)
        {
            return std::strncmp(s1.c_str(), s2.c_str(), l) == 0;
        }

        inline bool endswith(const std::string& str, const std::string& suffix)
        {
            return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
        }

        inline bool startswith(const std::string& str, const std::string& prefix)
        {
            return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
        }

        inline void to_upper(std::string& data)
        {
            std::for_each(data.begin(), data.end(), [](char & c){
                c = ::toupper(c);
            });
        }

        inline std::string to_upper_copy(const std::string& data)
        {
            std::string rtn = data;
            to_upper(rtn);
            return rtn;
        }

        inline void to_lower(std::string& data)
        {
            std::for_each(data.begin(), data.end(), [](char & c){
                c = ::tolower(c);
            });
        }

        inline std::string to_lower_copy(const std::string& data)
        {
            std::string rtn = data;
            to_lower(rtn);
            return rtn;
        }

        inline bool is_final_status(const OrderStatus &status)
        {
            switch (status)
            {
                case OrderStatus::Submitted:
                case OrderStatus::Pending:
                case OrderStatus::PartialFilledActive:
                case OrderStatus::Unknown:
                    return false;
                default:
                    return true;
            }
        }

        inline bool is_reverse_repurchase(const std::string &instrument_id, const std::string &exchange_id)
        {
            return (string_equals_n(instrument_id, "204", 3) && string_equals(exchange_id, EXCHANGE_SSE)) || (string_equals_n(instrument_id, "1318", 4) && string_equals(exchange_id, EXCHANGE_SZE));
        }

        inline double get_reverse_repurchase_commission_rate(const std::string &instrument_id)
        {
            if (string_equals(instrument_id, "204001") || string_equals(instrument_id, "131810"))
            {
                return 0.00001;
            } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811"))
            {
                return 0.00002;
            } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800"))
            {
                return 0.00003;
            } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809") )
            {
                return 0.00004;
            } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801"))
            {
                return 0.00005;
            } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802"))
            {
                return 0.00010;
            } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803"))
            {
                return 0.00020;
            } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805"))
            {
                return 0.00030;
            } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806"))
            {
                return 0.00030;
            } else
            {
                return 0.0;
            }
        }

        inline int get_reverse_repurchase_expire_days(const std::string &instrument_id)
        {
            if (string_equals(instrument_id, "204001")|| string_equals(instrument_id, "131810"))
            {
                return 1;
            } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811"))
            {
                return 2;
            } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800"))
            {
                return 3;
            } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809"))
            {
                return 4;
            } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801"))
            {
                return 7;
            } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802"))
            {
                return 14;
            } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803"))
            {
                return 28;
            } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805"))
            {
                return 91;
            } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806"))
            {
                return 182;
            } else
            {
                return 0;
            }
        }

        inline InstrumentType get_instrument_type(const std::string &instrument_id, const std::string &exchange_id)
        {
            if (string_equals(exchange_id, EXCHANGE_SSE))
            {
                if (instrument_id.size() == 8)
                {
                    return InstrumentType::StockOption;
                }
                else if(startswith(instrument_id, "00"))
                {
                    return InstrumentType::Index;
                }
                else if(startswith(instrument_id, "0"))
                {
                    return InstrumentType::Bond;
                }
                else if(startswith(instrument_id, "1"))
                {
                    return InstrumentType::Bond;
                }
                else if(startswith(instrument_id, "2"))
                {
                    return InstrumentType::Repo;
                }
                else if (startswith(instrument_id, "5"))
                {
                    return InstrumentType::Fund;
                }
                else if(startswith(instrument_id, "6"))
                {
                    return InstrumentType::Stock;
                }
            }
            else if(string_equals(exchange_id, EXCHANGE_SZE))
            {
                if (instrument_id.size() == 8)
                {
                    return InstrumentType::StockOption;
                }
                else if (startswith(instrument_id, "0"))
                {
                    return InstrumentType::Stock;
                }
                else if(startswith(instrument_id, "15") || startswith(instrument_id, "16") || startswith(instrument_id, "18"))
                {
                    return InstrumentType::Fund;
                }
                else if(startswith(instrument_id, "13"))
                {
                    return InstrumentType::Repo;
                }
                else if(startswith(instrument_id, "1"))
                {
                    return InstrumentType::Bond;
                }
                else if(startswith(instrument_id, "30"))
                {
                    return InstrumentType::Stock;
                }
            }
            else if(string_equals(exchange_id, EXCHANGE_DCE) || string_equals(exchange_id, EXCHANGE_SHFE) || string_equals(exchange_id, EXCHANGE_CFFEX) || string_equals(exchange_id, EXCHANGE_CZCE) || string_equals(exchange_id, EXCHANGE_INE))
            {
                return InstrumentType::Future;
            }
            return InstrumentType::Unknown;
        }

        inline std::string str_from_instrument_type(InstrumentType type)
        {
            switch(type)
            {
                case InstrumentType::Unknown:
                    return "Unknown";
                case InstrumentType::Stock:
                    return "Stock";
                case InstrumentType::Future:
                    return "Future";
                case InstrumentType::Bond:
                    return "Bond";
                case InstrumentType::StockOption:
                    return "StockOption";
                case InstrumentType::Fund:
                    return "Fund";
                case InstrumentType::TechStock:
                    return "TechStock";
                case InstrumentType::Index:
                    return "Index";
                case InstrumentType::Repo:
                    return "Repo";
                default:
                    return "Unknown";
            }
        }

        inline std::string get_instrument_product(const char *instrument_id)
        {
            std::string product = "";
            int i = 0;
            while (instrument_id[i] != 0)
            {
                if (instrument_id[i] < '0' || instrument_id[i] > '9')
                {
                    product.push_back(instrument_id[i]);
                }
                ++i;
            }
            return product;
        }

        inline Direction get_future_direction(Side side, Offset offset)
        {
            if ((side == Side::Buy && offset == Offset::Open) || (side == Side::Sell && offset != Offset::Open))
            {
                return Direction::Long;
            } else
            {
                return Direction::Short;
            }
        }

        inline bool is_open(InstrumentType instrument_type, Side side, Offset offset)
        {
            if (instrument_type == InstrumentType::Stock || instrument_type == InstrumentType::Bond)
            {
                return side == Side::Buy;
            } else if (instrument_type == InstrumentType::Future)
            {
                return offset == Offset::Open;
            } else
            {
                return false;
            }
        }

        inline std::string get_exchange_id_from_future_instrument_id(const std::string &instrument_id)
        {
            std::size_t found = instrument_id.find_first_of("0123456789");
            std::string product = instrument_id.substr(0, found);
            std::transform(product.begin(), product.end(), product.begin(), ::tolower);
            if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
                product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "rr" || product == "l" ||
                product == "v" || product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg" ||
                product == "eb")
            {
                return EXCHANGE_DCE;
            } else if (product == "wh" || product == "pm" || product == "cf" || product == "sr" || product == "oi" || product == "ri" ||
                       product == "rs" || product == "rm" || product == "jr" || product == "lr" || product == "cy" || product == "ap" ||
                       product == "cj" || product == "ta" || product == "ma" || product == "zc" || product == "sf" || product == "sm" ||
                       product == "ur")
            {
                return EXCHANGE_CZCE;
            } else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" || product == "sn" ||
                       product == "au" || product == "ag" || product == "rb" || product == "wr" || product == "hc" || product == "ss" ||
                       product == "fu" || product == "bu" || product == "ru" || product == "nr" || product == "sp")
            {
                return EXCHANGE_SHFE;
            } else if (product == "if" || product == "ic" || product == "ih" || product == "ts" || product == "tf" || product == "t")
            {
                return EXCHANGE_CFFEX;
            } else if (product == "sc")
            {
                return EXCHANGE_INE;
            } else
            {
                return "";
            }
        }

        inline std::string get_exchange_id_from_stock_instrument_id(const std::string &instrument_id)
        {
            int code = atoi(instrument_id.c_str());
            int head3 = code / 1000;
            switch (head3)
            {
                case 001:
                case 201:
                case 100:
                case 110:
                case 120:
                case 129:
                case 310:
                case 500:
                case 600:
                case 601:
                case 700:
                case 701:
                case 710:
                case 720:
                case 730:
                case 735:
                case 737:
                case 900:
                    return EXCHANGE_SSE;
                default:
                    return EXCHANGE_SZE;
            }
        }

        inline uint32_t get_symbol_id(const std::string &symbol, const std::string &exchange)
        {
            return yijinjing::util::hash_str_32(symbol) ^ yijinjing::util::hash_str_32(exchange);
        }
    }
}

#endif //WINGCHUN_COMMON_H
