//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_DATAPARSER_H
#define KUNGFU_DATAPARSER_H

#include <cstdio>
#include <cstring>

#include "xtp_api_struct.h"

#include <kungfu/time/timer.h>
#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/util/business_helper.h>

namespace kungfu
{
    namespace xtp
    {
        inline void from_xtp_timestamp(const int64_t& xtp_time, int64_t & nsec)
        {
            //YYYYMMDDHHMMSSsss -> nano seconds
            nsec = kungfu::yijinjing::parseTime(std::to_string(xtp_time).c_str(), "%Y%m%d%H%M%S") + xtp_time % 1000 * kungfu::yijinjing::NANOSECONDS_PER_MILLISECOND;
        }

        inline void from_xtp(const XTP_MARKET_TYPE& xtp_market_type, char* exchange_id)
        {
            if (xtp_market_type == XTP_MKT_SH_A)
            {
                strcpy(exchange_id, "SSE");
            }
            else if (xtp_market_type == XTP_MKT_SZ_A)
            {
                strcpy(exchange_id, "SZE");
            }
        }

        inline void to_xtp(XTP_MARKET_TYPE& xtp_market_type, const char* exchange_id)
        {
            if (!strcmp(exchange_id, "SSE"))
            {
                xtp_market_type = XTP_MKT_SH_A;
            }
            else if (!strcmp(exchange_id, "SZE"))
            {
                xtp_market_type = XTP_MKT_SZ_A;
            }
            else
            {
                xtp_market_type = XTP_MKT_UNKNOWN;
            }
        }

        inline void from_xtp(const XTP_EXCHANGE_TYPE& xtp_exchange_type, char* exchange_id)
        {
            if (xtp_exchange_type == XTP_EXCHANGE_SH)
            {
                strcpy(exchange_id, "SSE");
            }
            else if (xtp_exchange_type == XTP_EXCHANGE_SZ)
            {
                strcpy(exchange_id, "SZE");
            }
        }

        inline void to_xtp(XTP_EXCHANGE_TYPE& xtp_exchange_type, char* exchange_id)
        {
            if (strcmp(exchange_id, "SSE") == 0)
            {
                xtp_exchange_type = XTP_EXCHANGE_SH;
            }
            else if (strcmp(exchange_id, "SZE") == 0)
            {
                xtp_exchange_type = XTP_EXCHANGE_SZ;
            }
            else
            {
                xtp_exchange_type = XTP_EXCHANGE_UNKNOWN;
            }
        }

        inline void from_xtp(const XTP_PRICE_TYPE& xtp_price_type, PriceType& price_type, TimeCondition& time_condition, VolumeCondition& volume_condition)
        {
            if (xtp_price_type == XTP_PRICE_LIMIT )
            {
                price_type = PriceTypeLimit;
                time_condition = TimeConditionGFD;
                volume_condition = VolumeConditionAny;
            }
            else if (xtp_price_type == XTP_PRICE_BEST5_OR_CANCEL)
            {
                price_type = PriceTypeBest5;
                time_condition = TimeConditionIOC;
                volume_condition = VolumeConditionAny;
            }
        }

        inline void to_xtp(XTP_PRICE_TYPE& xtp_price_type,const PriceType& price_type, const TimeCondition& time_condition, const VolumeCondition& volume_condition)
        {
            if (price_type == PriceTypeLimit && time_condition == TimeConditionGFD) // 限价
            {
                xtp_price_type = XTP_PRICE_LIMIT;
            }
            else if (price_type == PriceTypeBest && time_condition == TimeConditionIOC) //
            {
                xtp_price_type = XTP_PRICE_BEST_OR_CANCEL;
            }
            else if (price_type == PriceTypeBest5 && time_condition == TimeConditionIOC) // 最优五档成交剩余转撤销
            {
                xtp_price_type = XTP_PRICE_BEST5_OR_CANCEL;
            }
            else if (price_type == PriceTypeBest5 && time_condition == TimeConditionGFD) // 最优五档成交剩余转限价
            {
                xtp_price_type = XTP_PRICE_BEST5_OR_LIMIT;
            }
            else if (price_type == PriceTypeForwardBest) //本方最优价
            {
                xtp_price_type = XTP_PRICE_FORWARD_BEST;
            }
            else if (price_type == PriceTypeReverseBest && time_condition == TimeConditionGFD)
            {
                xtp_price_type = XTP_PRICE_REVERSE_BEST_LIMIT;
            }
        }

        inline void from_xtp(const XTP_ORDER_STATUS_TYPE& xtp_order_status, OrderStatus& status)
        {
            if (xtp_order_status == XTP_ORDER_STATUS_INIT || xtp_order_status == XTP_ORDER_STATUS_NOTRADEQUEUEING)
            {
                status = OrderStatusPending;
            }
            else if (xtp_order_status == XTP_ORDER_STATUS_ALLTRADED)
            {
                status = OrderStatusFilled;
            }
            else if (xtp_order_status == XTP_ORDER_STATUS_CANCELED)
            {
                status = OrderStatusCancelled;
            }
            else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDQUEUEING)
            {
                status = OrderStatusPartialFilledActive;
            }
            else if (xtp_order_status == XTP_ORDER_STATUS_PARTTRADEDNOTQUEUEING)
            {
                status = OrderStatusPartialFilledNotActive;
            }
            else if (xtp_order_status == XTP_ORDER_STATUS_REJECTED)
            {
                status = OrderStatusError;
            }
            else
            {
                status = OrderStatusUnknown;
            }
        }

        inline void from_xtp(const XTP_SIDE_TYPE& xtp_side, Side& side)
        {
            if (xtp_side == XTP_SIDE_BUY)
            {
                side = SideBuy;
            }
            else if (xtp_side == XTP_SIDE_SELL)
            {
                side = SideSell;
            }
        }

        inline void to_xtp(XTP_SIDE_TYPE& xtp_side, const Side& side)
        {
            if (side == SideBuy)
            {
                xtp_side = XTP_SIDE_BUY;
            }
            else if (side == SideSell)
            {
                xtp_side = XTP_SIDE_SELL;
            }
        }

        inline void to_xtp(XTPMarketDataStruct& des, const Quote& ori)
        {
            //TODO
        }

        inline void from_xtp(const XTPMarketDataStruct& ori, Quote& des)
        {
            strcpy(des.source_id, SOURCE_XTP);
            strcpy(des.trading_day, yijinjing::parseNano(des.data_time, "%Y%m%d").c_str());
            from_xtp_timestamp(ori.data_time, des.data_time);
            strcpy(des.instrument_id, ori.ticker);
            from_xtp(ori.exchange_id, des.exchange_id);

            if (ori.data_type == XTP_MARKETDATA_OPTION)
            {
                des.instrument_type = InstrumentTypeStockOption;
            }
            else
            {
                // 目前除逆回购之外的债券均当做股票
                if (is_reverse_repurchase(des.instrument_id, des.exchange_id))
                {
                    des.instrument_type = InstrumentTypeBond;
                }
                else
                {
                    des.instrument_type = InstrumentTypeStock;
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

            for (int i =0; i < 10; i++)
            {
                des.ask_price[i] = ori.ask[i];
                des.ask_volume[i] = ori.ask_qty[i];
                des.bid_price[i] = ori.bid[i];
                des.bid_volume[i] = ori.bid_qty[i];
            }
        }

        inline void to_xtp(XTPOrderInsertInfo& des, const OrderInput& ori)
        {
            strcpy(des.ticker, ori.instrument_id);
            to_xtp(des.market, ori.exchange_id);
            des.price = ori.limit_price;
            des.quantity = ori.volume;
            to_xtp(des.side, ori.side);
            to_xtp(des.price_type, ori.price_type, ori.time_condition, ori.volume_condition);
            des.business_type = XTP_BUSINESS_TYPE_CASH;
        }

        inline void from_xtp(const XTPOrderInsertInfo& ori, OrderInput& des)
        {
            //TODO
        }

        inline void from_xtp(const XTPOrderInfo& ori, Order& des)
        {
            strcpy(des.instrument_id, ori.ticker);
            from_xtp(ori.market, des.exchange_id);
            from_xtp(ori.price_type, des.price_type, des.time_condition, des.volume_condition);
            des.volume = ori.quantity;
            des.volume_traded = ori.qty_traded;
            des.volume_left = ori.qty_left;
            des.limit_price = ori.price;
            from_xtp(ori.order_status, des.status);
            from_xtp(ori.side, des.side);
            des.offset = OffsetOpen;
            if (ori.business_type == XTP_BUSINESS_TYPE_CASH)
            {
                des.instrument_type = InstrumentTypeStock;
            }
            if (ori.update_time > 0)
            {
                from_xtp_timestamp(ori.update_time, des.update_time);
            }
        }

        inline void from_xtp(const XTPTradeReport& ori, Trade& des)
        {
            strcpy(des.instrument_id, ori.ticker);
            des.volume = ori.quantity;
            des.price = ori.price;
            from_xtp(ori.market, des.exchange_id);
            from_xtp(ori.side, des.side);
            des.offset = OffsetOpen;
            if (ori.business_type == XTP_BUSINESS_TYPE_CASH)
            {
                des.instrument_type = InstrumentTypeStock;
            }
            from_xtp_timestamp(ori.trade_time, des.trade_time);
        }

        inline void from_xtp(const XTPQueryStkPositionRsp& ori, Position& des)
        {
            strcpy(des.instrument_id, ori.ticker);
            from_xtp(ori.market, des.exchange_id);
            des.volume = ori.total_qty;
            des.yesterday_volume = ori.yesterday_position;
            des.cost_price = ori.avg_price;
        }

        inline void from_xtp(const XTPQueryAssetRsp& ori, AccountInfo& des)
        {
            des.avail = ori.buying_power;
        }

        inline void from_xtp(const XTPTickByTickStruct& ori, Entrust& des)
        {
            from_xtp(ori.exchange_id, des.exchange_id);
            strcpy(des.instrument_id, ori.ticker);
            from_xtp_timestamp(ori.data_time, des.data_time);

            des.price = ori.entrust.price;
            des.volume = ori.entrust.qty;
            des.main_seq = ori.entrust.channel_no;
            des.seq = ori.entrust.seq;

            if (ori.entrust.ord_type == '1')
            {
                des.price_type = PriceTypeAny;
            }
            else if (ori.entrust.ord_type == '2')
            {
                des.price_type = PriceTypeLimit;
            }
            else if (ori.entrust.ord_type == 'U')
            {
                des.price_type = PriceTypeBest;
            }
        }

        inline void from_xtp(const XTPTickByTickStruct& ori, Transaction& des)
        {
            strcpy(des.source_id, SOURCE_XTP);

            from_xtp(ori.exchange_id, des.exchange_id);
            strcpy(des.instrument_id, ori.ticker);
            from_xtp_timestamp(ori.data_time, des.data_time);

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
                    des.bs_flag = BsFlagBuy;
                    break;
                }
                case 'S':
                {
                    des.bs_flag = BsFlagSell;
                    break;
                }
                case 'N':
                {
                    des.bs_flag = BsFlagUnknown;
                    break;
                }
                case '4':
                {
                    des.exec_type = ExecTypeCancel;
                    break;
                }
                case 'F':
                {
                    des.exec_type = ExecTypeTrade;
                    break;
                }
                default:
                {
                    ;
                }
            }
        }
    }
}
#endif //KUNGFU_DATAPARSER_H
