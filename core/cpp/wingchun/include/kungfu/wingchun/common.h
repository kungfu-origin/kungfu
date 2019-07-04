//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define EXCHANGE_SSE "SSE"
#define EXCHANGE_SZE "SZE"
#define EXCHANGE_SHFE "SHFE"
#define EXCHANGE_DCE "DCE"
#define EXCHANGE_CZCE "CZCE"
#define EXCHANGE_CFFEX "CFFEX"
#define EXCHANGE_INE "INE"

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
    }
}

#endif //WINGCHUN_COMMON_H
