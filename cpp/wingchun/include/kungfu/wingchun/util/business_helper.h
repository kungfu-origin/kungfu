//
// Created by PolarAir on 2019-02-21.
//
// 与金融业务有关的一些简单公共函数
//

#ifndef KUNGFU_BUSINESS_HELPER_H
#define KUNGFU_BUSINESS_HELPER_H

#include <string.h>
#include <stdio.h>
#include <cmath>

#include <kungfu/time/timer.h>

#include <kungfu/wingchun/constant.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace kungfu
{
    #define EPSILON (1e-6)
    #define DOUBLEMAX (1e16) // 一亿亿, 2018年A股总市值不到50万亿

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

    inline double rounded(double x, int n)
    {
        if (is_too_large(x) || is_zero(x) || is_too_large(std::abs(x)))
        {
            return 0.0;
        }
        else
        {
            char out[64];
            double xrounded;
            sprintf(out, "%.*f", n, x);
            xrounded = strtod(out, 0);
            return xrounded;
        }
    }

    inline bool is_final_status(const OrderStatus& status)
    {
        switch (status)
        {
            case OrderStatusSubmitted:
            case OrderStatusPending:
            case OrderStatusPartialFilledNotActive:
            case OrderStatusUnknown:
                return false;
            default:
                return true;
        }
    }

    inline std::string get_symbol(const char* instrument_id, const char* exchange_id)
    {
        return std::string(instrument_id) + "." + std::string(exchange_id);
    }

    inline std::string get_symbol(const std::string& instrument_id, const std::string& exchange_id)
    {
        return get_symbol(instrument_id.c_str(), exchange_id.c_str());
    }

    inline journal::Order get_order(const journal::OrderInput& input)
    {
        journal::Order order = {};
        order.order_id = input.order_id;
        strcpy(order.instrument_id, input.instrument_id);
        strcpy(order.exchange_id, input.exchange_id);
        strcpy(order.account_id, input.account_id);
        strcpy(order.client_id, input.client_id);

        order.limit_price = input.limit_price;
        order.frozen_price = input.frozen_price;

        order.volume = input.volume;
        order.volume_traded = 0;
        order.volume_left = input.volume;
        order.status = OrderStatusSubmitted;

        order.parent_id = input.parent_id;

        order.side = input.side;
        order.offset = input.offset;

        order.price_type = input.price_type;
        order.volume_condition = input.volume_condition;
        order.time_condition = input.time_condition;

        order.parent_id = input.parent_id;

        return order;
    }

    inline bool is_reverse_repurchase(const char* instrument_id, const char* exchange_id)
    {
        return (strncmp(instrument_id, "204", 3) == 0 && strcmp(exchange_id, EXCHANGE_SSE) == 0) ||
                (strncmp(instrument_id, "1318", 4) == 0 && strcmp(exchange_id, EXCHANGE_SZE) == 0);
    }

    inline double get_reverse_repurchase_commission_rate(const char* instrument_id)
    {
        if (strcmp(instrument_id, "204001") == 0 || strcmp(instrument_id, "131810") == 0)
        {
            return 0.00001;
        }
        else if (strcmp(instrument_id, "204002") == 0 || strcmp(instrument_id, "131811") == 0)
        {
            return 0.00002;
        }
        else if (strcmp(instrument_id, "204003") == 0 || strcmp(instrument_id, "131800") == 0)
        {
            return 0.00003;
        }
        else if (strcmp(instrument_id, "204004") == 0 || strcmp(instrument_id, "131809") == 0)
        {
            return 0.00004;
        }
        else if (strcmp(instrument_id, "204007") == 0 || strcmp(instrument_id, "131801") == 0)
        {
            return 0.00005;
        }
        else if (strcmp(instrument_id, "204014") == 0 || strcmp(instrument_id, "131802") == 0)
        {
            return 0.00010;
        }
        else if (strcmp(instrument_id, "204029") == 0 || strcmp(instrument_id, "131803") == 0)
        {
            return 0.00020;
        }
        else if (strcmp(instrument_id, "204091") == 0 || strcmp(instrument_id, "131805") == 0)
        {
            return 0.00030;
        }
        else if (strcmp(instrument_id, "204182") == 0 || strcmp(instrument_id, "131806") == 0)
        {
            return 0.00030;
        }
        else
        {
            return 0.0;
        }
    }

    inline int get_reverse_repurchase_expire_days(const char* instrument_id)
    {
        if (strcmp(instrument_id, "204001") == 0 || strcmp(instrument_id, "131810") == 0)
        {
            return 1;
        }
        else if (strcmp(instrument_id, "204002") == 0 || strcmp(instrument_id, "131811") == 0)
        {
            return 2;
        }
        else if (strcmp(instrument_id, "204003") == 0 || strcmp(instrument_id, "131800") == 0)
        {
            return 3;
        }
        else if (strcmp(instrument_id, "204004") == 0 || strcmp(instrument_id, "131809") == 0)
        {
            return 4;
        }
        else if (strcmp(instrument_id, "204007") == 0 || strcmp(instrument_id, "131801") == 0)
        {
            return 7;
        }
        else if (strcmp(instrument_id, "204014") == 0 || strcmp(instrument_id, "131802") == 0)
        {
            return 14;
        }
        else if (strcmp(instrument_id, "204029") == 0 || strcmp(instrument_id, "131803") == 0)
        {
            return 28;
        }
        else if (strcmp(instrument_id, "204091") == 0 || strcmp(instrument_id, "131805") == 0)
        {
            return 91;
        }
        else if (strcmp(instrument_id, "204182") == 0 || strcmp(instrument_id, "131806") == 0)
        {
            return 182;
        }
        else
        {
            return 0;
        }
    }

    inline InstrumentType get_instrument_type(const char* instrument_id, const char* exchange_id)
    {
        if (strcmp(exchange_id, EXCHANGE_SSE) == 0 || strcmp(exchange_id, EXCHANGE_SZE) == 0)
        {
            if (is_reverse_repurchase(instrument_id, exchange_id))
            {
                return InstrumentTypeBond;
            }
            else
            {
                return InstrumentTypeStock;
            }
        }
        else
        {
            return InstrumentTypeFuture;
        }
    }

    inline Direction get_future_direction(Side side, Offset offset)
    {
        if ((side == SideBuy && offset == OffsetOpen) || (side == SideSell && offset != OffsetOpen))
        {
            return DirectionLong;
        }
        else
        {
            return DirectionShort;
        }
    }

    inline std::string get_instrument_product(const char *instrument_id)
    {
        int pos = 0;
        while((instrument_id[pos]>'a'&&instrument_id[pos]<'z')||(instrument_id[pos]>'A'&&instrument_id[pos]<'Z'))
        {
            pos++;
        }
        return std::string(instrument_id, pos);
    }

    inline std::string get_reverse_repurchase_expire_date(const char* instrument_id, const char* open_date)
    {
        static Calendar s_calendar;
        return s_calendar.get_next_trading_day(open_date, get_reverse_repurchase_expire_days(instrument_id));
    }

    inline bool is_open(InstrumentType instrument_type, Side side, Offset offset)
    {
        if (instrument_type == InstrumentTypeStock || instrument_type == InstrumentTypeBond)
        {
            return side == SideBuy;
        }
        else if (instrument_type == InstrumentTypeFuture)
        {
            return offset == OffsetOpen;
        }
        else
        {
            return false;
        }
    }

    inline std::string get_exchange_id_from_future_instrument_id(const char* instrument_id)
    {
        std::string product = std::string(instrument_id).substr(0, strlen(instrument_id) - 4);
        std::transform(product.begin(), product.end(), product.begin(), ::tolower);
        if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
            product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "l" || product == "v" ||
            product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg")
        {
            return EXCHANGE_DCE;
        }
        else if (product == "sr" || product == "cf" || product == "pm" || product == "wh" || product == "ri" || product == "lr" ||
                product == "jr" || product == "rm" || product == "rs" || product == "rs" || product == "rm" || product == "oi" ||
                product == "cy" || product == "ta" || product == "ma" || product == "fg" || product == "sf" || product == "sm")
        {
            return EXCHANGE_CZCE;
        }
        else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" || product == "sn" ||
                product == "au" || product == "ag" || product == "rb" || product == "wr" || product == "hc" || product == "fu" ||
                product == "bu" || product == "ru" || product == "sp")
        {
            return EXCHANGE_SHFE;
        }
        else if (product == "if" || product == "ic" || product == "ih" || product == "tf" || product == "tf" || product == "t")
        {
            return EXCHANGE_CFFEX;
        }
        else if (product == "sc")
        {
            return EXCHANGE_INE;
        }
        else
        {
            return "";
        }
    }

    inline std::string get_exchange_id_from_stock_instrument_id(const char* instrument_id)
    {
        int code = atoi(instrument_id);
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

#endif //KUNGFU_BUSINESS_HELPER_H
