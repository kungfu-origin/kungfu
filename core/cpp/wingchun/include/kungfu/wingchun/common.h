//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>
#include <string>
#include <fmt/format.h>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define EXCHANGE_SSE "SSE"
#define EXCHANGE_SZE "SZE"
#define EXCHANGE_SHFE "SHFE"
#define EXCHANGE_DCE "DCE"
#define EXCHANGE_CZCE "CZCE"
#define EXCHANGE_CFFEX "CFFEX"
#define EXCHANGE_INE "INE"

#define SOURCE_PASSIVE "passive"
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
        const int ERROR_MSG_LEN = 32;

        enum class InstrumentType: int8_t
        {
            Unknown,
            Stock,
            Future,
            Bond,
            StockOption
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
            Sell
        };

        enum class Offset: int8_t
        {
            Open,
            Close,
            CloseToday,
            CloseYesterday
        };

        enum class OrderActionFlag: int8_t
        {
            Cancel,
        };

        enum class PriceType: int8_t
        {
            Any,
            Best,
            Best5,
            Limit,
            ForwardBest,
            ReverseBest
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
            Portfolio,
            SubPortfolio,
            Unknown
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

        inline bool is_final_status(const OrderStatus &status)
        {
            switch (status)
            {
                case OrderStatus::Submitted:
                case OrderStatus::Pending:
                case OrderStatus::PartialFilledNotActive:
                case OrderStatus::Unknown:
                    return false;
                default:
                    return true;
            }
        }

        inline bool is_reverse_repurchase(const std::string &instrument_id, const std::string &exchange_id)
        {
            return (string_equals_n(instrument_id, "204", 3) == 0 && string_equals(exchange_id, EXCHANGE_SSE) == 0) ||
                   (string_equals_n(instrument_id, "1318", 4) == 0 && string_equals(exchange_id, EXCHANGE_SZE) == 0);
        }

        inline double get_reverse_repurchase_commission_rate(const std::string &instrument_id)
        {
            if (string_equals(instrument_id, "204001") == 0 || string_equals(instrument_id, "131810") == 0)
            {
                return 0.00001;
            } else if (string_equals(instrument_id, "204002") == 0 || string_equals(instrument_id, "131811") == 0)
            {
                return 0.00002;
            } else if (string_equals(instrument_id, "204003") == 0 || string_equals(instrument_id, "131800") == 0)
            {
                return 0.00003;
            } else if (string_equals(instrument_id, "204004") == 0 || string_equals(instrument_id, "131809") == 0)
            {
                return 0.00004;
            } else if (string_equals(instrument_id, "204007") == 0 || string_equals(instrument_id, "131801") == 0)
            {
                return 0.00005;
            } else if (string_equals(instrument_id, "204014") == 0 || string_equals(instrument_id, "131802") == 0)
            {
                return 0.00010;
            } else if (string_equals(instrument_id, "204029") == 0 || string_equals(instrument_id, "131803") == 0)
            {
                return 0.00020;
            } else if (string_equals(instrument_id, "204091") == 0 || string_equals(instrument_id, "131805") == 0)
            {
                return 0.00030;
            } else if (string_equals(instrument_id, "204182") == 0 || string_equals(instrument_id, "131806") == 0)
            {
                return 0.00030;
            } else
            {
                return 0.0;
            }
        }

        inline int get_reverse_repurchase_expire_days(const std::string &instrument_id)
        {
            if (string_equals(instrument_id, "204001") == 0 || string_equals(instrument_id, "131810") == 0)
            {
                return 1;
            } else if (string_equals(instrument_id, "204002") == 0 || string_equals(instrument_id, "131811") == 0)
            {
                return 2;
            } else if (string_equals(instrument_id, "204003") == 0 || string_equals(instrument_id, "131800") == 0)
            {
                return 3;
            } else if (string_equals(instrument_id, "204004") == 0 || string_equals(instrument_id, "131809") == 0)
            {
                return 4;
            } else if (string_equals(instrument_id, "204007") == 0 || string_equals(instrument_id, "131801") == 0)
            {
                return 7;
            } else if (string_equals(instrument_id, "204014") == 0 || string_equals(instrument_id, "131802") == 0)
            {
                return 14;
            } else if (string_equals(instrument_id, "204029") == 0 || string_equals(instrument_id, "131803") == 0)
            {
                return 28;
            } else if (string_equals(instrument_id, "204091") == 0 || string_equals(instrument_id, "131805") == 0)
            {
                return 91;
            } else if (string_equals(instrument_id, "204182") == 0 || string_equals(instrument_id, "131806") == 0)
            {
                return 182;
            } else
            {
                return 0;
            }
        }

        inline InstrumentType get_instrument_type(const std::string &instrument_id, const std::string &exchange_id)
        {
            if (string_equals(exchange_id, EXCHANGE_SSE) || string_equals(exchange_id, EXCHANGE_SZE))
            {
                if (is_reverse_repurchase(instrument_id, exchange_id))
                {
                    return InstrumentType::Bond;
                }
                else
                {
                    return InstrumentType::Stock;
                }
            }
            else
            {
                return InstrumentType::Future;
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

        inline std::string get_exchange_id_from_future_instrument_id(const char *instrument_id)
        {
            std::string product = get_instrument_product(instrument_id);
            std::transform(product.begin(), product.end(), product.begin(), ::tolower);
            if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
                product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "l" || product == "v" ||
                product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg" ||
                product.substr(0, 2) == "sp" || product.substr(0, 3) == "spc")
            {
                return EXCHANGE_DCE;
            } else if (product == "sr" || product == "cf" || product == "pm" || product == "wh" || product == "ri" || product == "lr" ||
                       product == "jr" || product == "rm" || product == "rs" || product == "rs" || product == "rm" || product == "oi" ||
                       product == "cy" || product == "ta" || product == "ma" || product == "fg" || product == "sf" || product == "sm" ||
                       product.substr(0, 3) == "spd" || product.substr(0, 3) == "ips")
            {
                int pos = 0;
                while ((instrument_id[pos] > 'a' && instrument_id[pos] < 'z') || (instrument_id[pos] > 'A' && instrument_id[pos] < 'Z'))
                {
                    pos++;
                }
                return std::string(instrument_id, pos);
            }
            throw wingchun_error(fmt::format("unknown instrument [{}]", instrument_id));
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
            std::string product = std::string(instrument_id).substr(0, instrument_id.length() - 4);
            std::transform(product.begin(), product.end(), product.begin(), ::tolower);
            if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
                product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "l" || product == "v" ||
                product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg")
            {
                return EXCHANGE_DCE;
            } else if (product == "sr" || product == "cf" || product == "pm" || product == "wh" || product == "ri" || product == "lr" ||
                       product == "jr" || product == "rm" || product == "rs" || product == "rs" || product == "rm" || product == "oi" ||
                       product == "cy" || product == "ta" || product == "ma" || product == "fg" || product == "sf" || product == "sm")
            {
                return EXCHANGE_CZCE;
            } else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" || product == "sn" ||
                       product == "au" || product == "ag" || product == "rb" || product == "wr" || product == "hc" || product == "fu" ||
                       product == "bu" || product == "ru" || product == "sp")
            {
                return EXCHANGE_SHFE;
            } else if (product == "if" || product == "ic" || product == "ih" || product == "tf" || product == "tf" || product == "t")
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
    }
}

#endif //WINGCHUN_COMMON_H
