//
// Created by PolarAir on 2019-02-21.
//
// 与金融业务有关的一些简单公共函数
//

#ifndef KUNGFU_BUSINESS_HELPER_H
#define KUNGFU_BUSINESS_HELPER_H

#include <string>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/io.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/calendar/calendar.h>

namespace kungfu
{
    namespace wingchun
    {

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

        inline std::string get_symbol(const std::string &instrument_id, const std::string &exchange_id)
        {
            return instrument_id + "." + exchange_id;
        }

        inline msg::data::Order get_order(const msg::data::OrderInput &input, msg::data::Order &order, uint64_t external_id)
        {
            order.parent_id = input.parent_id;
            order.order_id = input.order_id;
            order.external_id = external_id;

            strcpy(order.instrument_id, input.instrument_id);
            strcpy(order.exchange_id, input.exchange_id);
            strcpy(order.account_id, input.account_id);
//            strcpy(order.client_id, input.client_id);

            order.limit_price = input.limit_price;
            order.frozen_price = input.frozen_price;

            order.volume = input.volume;
            order.volume_traded = 0;
            order.volume_left = input.volume;
            order.status = OrderStatus::Submitted;

            order.side = input.side;
            order.offset = input.offset;

            order.price_type = input.price_type;
            order.volume_condition = input.volume_condition;
            order.time_condition = input.time_condition;

            return order;
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
#endif //KUNGFU_BUSINESS_HELPER_H
