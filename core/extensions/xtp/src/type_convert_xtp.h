//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_XTP_EXT_TYPE_CONVERT_H
#define KUNGFU_XTP_EXT_TYPE_CONVERT_H

#include <cstdio>
#include <cstring>
#include <ctime>

#include "xtp_api_struct.h"

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace xtp
        {
            using namespace kungfu::wingchun::msg::data;
            inline XTP_PROTOCOL_TYPE get_xtp_protocol_type(const std::string &p)
            {
                if (p == "udp")
                {
                    return XTP_PROTOCOL_UDP;
                }
                else
                {
                    return XTP_PROTOCOL_TCP;
                }
            }

            inline int64_t nsec_from_xtp_timestamp(int64_t xtp_time)
            {
                std::tm result = {};
                result.tm_year =  xtp_time / (int64_t)1e13 - 1900;
                result.tm_mon = xtp_time % (int64_t)1e13 / (int64_t)1e11 -1;
                result.tm_mday = xtp_time % (int64_t)1e11 / (int64_t)1e9;
                result.tm_hour = xtp_time % (int64_t)1e9 / (int64_t)1e7;
                result.tm_min =  xtp_time % (int)1e7 / (int)1e5;
                result.tm_sec = xtp_time % (int)1e5 /(int)1e3;
                int milli_sec = xtp_time %(int)1e3;
                std::time_t parsed_time = std::mktime(&result);
                int64_t nano = parsed_time * kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND;
                return parsed_time * kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND + milli_sec * kungfu::yijinjing::time_unit::NANOSECONDS_PER_MILLISECOND;
            }

            inline void from_xtp(const XTP_MARKET_TYPE &xtp_market_type, char *exchange_id)
            {
                if (xtp_market_type == XTP_MKT_SH_A)
                {
                    strcpy(exchange_id, "SSE");
                } else if (xtp_market_type == XTP_MKT_SZ_A)
                {
                    strcpy(exchange_id, "SZE");
                }
            }

            inline void to_xtp(XTP_MARKET_TYPE &xtp_market_type, const char *exchange_id)
            {
                if (!strcmp(exchange_id, "SSE"))
                {
                    xtp_market_type = XTP_MKT_SH_A;
                } else if (!strcmp(exchange_id, "SZE"))
                {
                    xtp_market_type = XTP_MKT_SZ_A;
                } else
                {
                    xtp_market_type = XTP_MKT_UNKNOWN;
                }
            }

            inline std::string exchange_id_from_xtp(const XTP_EXCHANGE_TYPE ex)
            {
                if (ex == XTP_EXCHANGE_SH)
                {
                    return EXCHANGE_SSE;
                }
                else if(ex == XTP_EXCHANGE_SZ)
                {
                    return EXCHANGE_SZE;
                }
                else
                {
                    return "Unknown";
                }
            }

            inline void from_xtp(const XTP_EXCHANGE_TYPE &xtp_exchange_type, char *exchange_id)
            {
                if (xtp_exchange_type == XTP_EXCHANGE_SH)
                {
                    strcpy(exchange_id, "SSE");
                } else if (xtp_exchange_type == XTP_EXCHANGE_SZ)
                {
                    strcpy(exchange_id, "SZE");
                }
            }

            inline void to_xtp(XTP_EXCHANGE_TYPE &xtp_exchange_type, char *exchange_id)
            {
                if (strcmp(exchange_id, "SSE") == 0)
                {
                    xtp_exchange_type = XTP_EXCHANGE_SH;
                } else if (strcmp(exchange_id, "SZE") == 0)
                {
                    xtp_exchange_type = XTP_EXCHANGE_SZ;
                } else
                {
                    xtp_exchange_type = XTP_EXCHANGE_UNKNOWN;
                }
            }

            inline void from_xtp(const XTP_PRICE_TYPE &xtp_price_type, const  XTP_MARKET_TYPE &xtp_exchange_type, PriceType &price_type)
            {
                if (xtp_price_type == XTP_PRICE_LIMIT)
                    price_type = PriceType::Limit;
                else if (xtp_price_type == XTP_PRICE_BEST5_OR_CANCEL)
                    price_type = PriceType::FakBest5;
                else if (xtp_exchange_type == XTP_MKT_SH_A)
                {
                    if (xtp_price_type == XTP_PRICE_BEST5_OR_LIMIT)
                        price_type = PriceType::ReverseBest;
                }
                else if(xtp_exchange_type == XTP_MKT_SZ_A)
                {
                    if (xtp_price_type == XTP_PRICE_BEST_OR_CANCEL)
                        price_type = PriceType::Fak;
                    else if (xtp_price_type == XTP_PRICE_FORWARD_BEST)
                        price_type = PriceType::ForwardBest;
                    else if (xtp_price_type == XTP_PRICE_REVERSE_BEST_LIMIT)
                        price_type = PriceType::ReverseBest;
                }
                else
                    price_type = PriceType::UnKnown;
            }

            inline void to_xtp(XTP_PRICE_TYPE &xtp_price_type, const PriceType &price_type, const char* exchange)
            {
                if (price_type == PriceType::Limit)
                    xtp_price_type = XTP_PRICE_LIMIT;
                else if ((price_type == PriceType::Any) || (price_type == PriceType::FakBest5))
                    xtp_price_type = XTP_PRICE_BEST5_OR_CANCEL;
                else if (strcmp(exchange, EXCHANGE_SSE) == 0)
                {
                    if (price_type == PriceType::ReverseBest)
                        xtp_price_type = XTP_PRICE_BEST5_OR_LIMIT;
                }
                else if (strcmp(exchange, EXCHANGE_SZE) == 0)
                {
                    if (price_type == PriceType::Fak)
                        xtp_price_type = XTP_PRICE_BEST_OR_CANCEL;
                    else if (price_type == PriceType::ForwardBest)
                        xtp_price_type = XTP_PRICE_FORWARD_BEST;
                    else if (price_type == PriceType::ReverseBest)
                        xtp_price_type = XTP_PRICE_REVERSE_BEST_LIMIT;
                }
                else
                    xtp_price_type = XTP_PRICE_TYPE_UNKNOWN;
            }

            inline void from_xtp(const XTP_ORDER_STATUS_TYPE &xtp_order_status, OrderStatus &status)
            {
                if (xtp_order_status == XTP_ORDER_STATUS_INIT || xtp_order_status == XTP_ORDER_STATUS_NOTRADEQUEUEING)
                {
                    status = OrderStatus::Pending;
                } else if (xtp_order_status == XTP_ORDER_STATUS_ALLTRADED)
                {
                    status = OrderStatus::Filled;
                } else if (xtp_order_status == XTP_ORDER_STATUS_CANCELED)
                {
                    status = OrderStatus::Cancelled;
                } else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDQUEUEING)
                {
                    status = OrderStatus::PartialFilledActive;
                } else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDNOTQUEUEING)
                {
                    status = OrderStatus::PartialFilledNotActive;
                } else if (xtp_order_status == XTP_ORDER_STATUS_REJECTED)
                {
                    status = OrderStatus::Error;
                } else
                {
                    status = OrderStatus::Unknown;
                }
            }

            inline void from_xtp(const XTP_SIDE_TYPE &xtp_side, Side &side)
            {
                if (xtp_side == XTP_SIDE_BUY)
                {
                    side = Side::Buy;
                } else if (xtp_side == XTP_SIDE_SELL)
                {
                    side = Side::Sell;
                }
            }

            inline void to_xtp(XTP_SIDE_TYPE &xtp_side, const Side &side)
            {
                if (side == Side::Buy)
                {
                    xtp_side = XTP_SIDE_BUY;
                } else if (side == Side::Sell)
                {
                    xtp_side = XTP_SIDE_SELL;
                }
            }

            inline void to_xtp(XTPMarketDataStruct &des, const Quote &ori)
            {
                //TODO
            }

            inline void from_xtp(const XTPMarketDataStruct &ori, Quote &des)
            {
                strcpy(des.source_id, SOURCE_XTP);
                des.data_time = nsec_from_xtp_timestamp(ori.data_time);
                strcpy(des.trading_day, yijinjing::time::strftime(des.data_time, "%Y%m%d").c_str());
                strcpy(des.instrument_id, ori.ticker);
                from_xtp(ori.exchange_id, des.exchange_id);

                if (ori.data_type == XTP_MARKETDATA_OPTION)
                {
                    des.instrument_type = InstrumentType::StockOption;
                } else
                {
                    // 目前除逆回购之外的债券均当做股票
                    if (is_reverse_repurchase(des.instrument_id, des.exchange_id))
                    {
                        des.instrument_type = InstrumentType::Bond;
                    } else
                    {
                        des.instrument_type = InstrumentType::Stock;
                    }
                }

                des.last_price = ori.last_price;
                des.pre_settlement_price = ori.pre_settl_price;
                des.pre_close_price = ori.pre_close_price;
                des.open_price = ori.open_price;
                des.high_price = ori.high_price;
                des.low_price = ori.low_price;
                des.volume = ori.qty;
                des.turnover = ori.turnover;
                des.close_price = ori.close_price;
                des.settlement_price = ori.settl_price;
                des.upper_limit_price = ori.upper_limit_price;
                des.lower_limit_price = ori.lower_limit_price;

                for (int i = 0; i < 10; i++)
                {
                    des.ask_price[i] = ori.ask[i];
                    des.ask_volume[i] = ori.ask_qty[i];
                    des.bid_price[i] = ori.bid[i];
                    des.bid_volume[i] = ori.bid_qty[i];
                }
            }

            inline void to_xtp(XTPOrderInsertInfo &des, const OrderInput &ori)
            {
                strcpy(des.ticker, ori.instrument_id);
                to_xtp(des.market, ori.exchange_id);
                des.price = ori.limit_price;
                des.quantity = ori.volume;
                to_xtp(des.side, ori.side);
                to_xtp(des.price_type, ori.price_type, ori.exchange_id);
                des.business_type = XTP_BUSINESS_TYPE_CASH;
            }

            inline void from_xtp(const XTPOrderInsertInfo &ori, OrderInput &des)
            {
                //TODO
            }

            inline void from_xtp(const XTPOrderInfo &ori, Order &des)
            {
                strcpy(des.instrument_id, ori.ticker);
                from_xtp(ori.market, des.exchange_id);
                from_xtp(ori.price_type, ori.market, des.price_type);
                des.volume = ori.quantity;
                des.volume_traded = ori.qty_traded;
                des.volume_left = ori.qty_left;
                des.limit_price = ori.price;
                from_xtp(ori.order_status, des.status);
                from_xtp(ori.side, des.side);
                des.offset = Offset::Open;
                if (ori.business_type == XTP_BUSINESS_TYPE_CASH)
                {
                    des.instrument_type = InstrumentType::Stock;
                }
                if (ori.update_time > 0)
                {
                    des.update_time = nsec_from_xtp_timestamp(ori.update_time);
                }
            }

            inline void from_xtp(const XTPTradeReport &ori, Trade &des)
            {
                strcpy(des.instrument_id, ori.ticker);
                des.volume = ori.quantity;
                des.price = ori.price;
                from_xtp(ori.market, des.exchange_id);
                from_xtp(ori.side, des.side);
                des.offset = Offset::Open;
                if (ori.business_type == XTP_BUSINESS_TYPE_CASH)
                {
                    des.instrument_type = InstrumentType::Stock;
                }
                des.trade_time = nsec_from_xtp_timestamp(ori.trade_time);
            }

            inline void from_xtp(const XTPQueryStkPositionRsp &ori, Position &des)
            {
                strcpy(des.instrument_id, ori.ticker);
                from_xtp(ori.market, des.exchange_id);
                des.volume = ori.total_qty;
                des.yesterday_volume = ori.sellable_qty;
                des.position_cost_price = ori.avg_price;
            }

            inline void from_xtp(const XTPQueryAssetRsp &ori, Asset &des)
            {
                strcpy(des.source_id, SOURCE_XTP);
                des.avail = ori.buying_power;
            }

            inline void from_xtp(const XTPTickByTickStruct &ori, Entrust &des)
            {
                from_xtp(ori.exchange_id, des.exchange_id);
                strcpy(des.instrument_id, ori.ticker);
                des.data_time = nsec_from_xtp_timestamp(ori.data_time);

                des.price = ori.entrust.price;
                des.volume = ori.entrust.qty;
                des.main_seq = ori.entrust.channel_no;
                des.seq = ori.entrust.seq;

                if (ori.entrust.ord_type == '1')
                {
                    des.price_type = PriceType::Any;
                } else if (ori.entrust.ord_type == '2')
                {
                    des.price_type = PriceType::Limit;
                } else if (ori.entrust.ord_type == 'U')
                {
                    des.price_type = PriceType::ForwardBest;
                }
            }

            inline void from_xtp(const XTPTickByTickStruct &ori, Transaction &des)
            {
                strcpy(des.source_id, SOURCE_XTP);

                from_xtp(ori.exchange_id, des.exchange_id);
                strcpy(des.instrument_id, ori.ticker);
                des.data_time = nsec_from_xtp_timestamp(ori.data_time);

                des.main_seq = ori.trade.channel_no;
                des.seq = ori.trade.seq;

                des.price = ori.trade.price;
                des.volume = ori.trade.qty;

                des.bid_no = ori.trade.bid_no;
                des.ask_no = ori.trade.ask_no;

                switch (ori.trade.trade_flag)
                {
                    case 'B':
                    {
                        des.bs_flag = BsFlag::Buy;
                        break;
                    }
                    case 'S':
                    {
                        des.bs_flag = BsFlag::Sell;
                        break;
                    }
                    case 'N':
                    {
                        des.bs_flag = BsFlag::Unknown;
                        break;
                    }
                    case '4':
                    {
                        des.exec_type = ExecType::Cancel;
                        break;
                    }
                    case 'F':
                    {
                        des.exec_type = ExecType::Trade;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
}
#endif //KUNGFU_XTP_EXT_TYPE_CONVERT_H
