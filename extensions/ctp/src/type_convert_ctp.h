//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_DATAPARSER_H
#define KUNGFU_DATAPARSER_H

#include <stdio.h>
#include <cstring>

#include "ThostFtdcUserApiStruct.h"

#include <kungfu/time/timer.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>

#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/util/code_convert.h>
#include <kungfu/wingchun/util/instrument.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    namespace ctp
    {
         inline void to_ctp_comb_offset(TThostFtdcCombOffsetFlagType ctp_offset, const Offset& offset)
        {
            if (offset == OffsetClose)
            {
                ctp_offset[0] = THOST_FTDC_OF_Close;
            }
            else if (offset == OffsetOpen)
            {
                ctp_offset[0] = THOST_FTDC_OF_Open;
            }
            else if (offset == OffsetCloseToday)
            {
                ctp_offset[0] = THOST_FTDC_OF_CloseToday;
            }
            else if (offset == OffsetCloseYesterday)
            {
                ctp_offset[0] = THOST_FTDC_OF_CloseYesterday;
            }
        }
        
        inline void from_ctp_comb_offset(const TThostFtdcCombOffsetFlagType ctp_offset, Offset& offset)
        {
            if (ctp_offset[0] == THOST_FTDC_OF_Close)
            {
                offset = OffsetClose;
            }
            else if (ctp_offset[0] == THOST_FTDC_OF_Open)
            {
                offset = OffsetOpen;
            }
            else if (ctp_offset[0] == THOST_FTDC_OF_CloseToday)
            {
                offset = OffsetCloseToday;
            }
            else if (ctp_offset[0] == THOST_FTDC_OF_CloseYesterday)
            {
                offset =OffsetCloseYesterday;
            } 
        }

        inline void from_ctp_offset(const TThostFtdcOffsetFlagType ctp_offset, Offset& offset)
        {
            if (ctp_offset == THOST_FTDC_OF_Close)
            {
                offset = OffsetClose;
            }
            else if (ctp_offset == THOST_FTDC_OF_Open)
            {
                offset = OffsetOpen;
            }
            else if (ctp_offset == THOST_FTDC_OF_CloseToday)
            {
                offset = OffsetCloseToday;
            }
            else if (ctp_offset == THOST_FTDC_OF_CloseYesterday)
            {
                offset = OffsetCloseYesterday;
            }
        }

        inline void to_ctp_direction(TThostFtdcDirectionType& ctp_direction, const Side& side)
        {
            if (side == SideBuy)
            {
                ctp_direction = THOST_FTDC_D_Buy;
            }
            else if (side == SideSell)
            {
                ctp_direction = THOST_FTDC_D_Sell;
            }
        }

        inline void from_ctp_direction(const TThostFtdcDirectionType& ctp_direction, Side& side)
        {
            if (ctp_direction == THOST_FTDC_D_Buy)
            {
                side = SideBuy;
            }
            else if (ctp_direction == THOST_FTDC_D_Sell)
            {
                side = SideSell;
            }   
        }

        inline void from_ctp_pos_direction(const TThostFtdcPosiDirectionType& pos_direction, Direction& direction)
        {
            if (pos_direction == THOST_FTDC_PD_Long)
            {
                direction = DirectionLong;
            }
            else if (pos_direction == THOST_FTDC_PD_Short)
            {
                direction = DirectionShort;
            }
        }

        inline void to_ctp_price_type(TThostFtdcOrderPriceTypeType& des, const PriceType& ori)
        {
            if (ori == PriceTypeLimit)
            {
                des = THOST_FTDC_OPT_LimitPrice;
            }
            else if (ori == PriceTypeAny)
            {
                des = THOST_FTDC_OPT_AnyPrice;
            }
        }

        inline void from_ctp_price_type(const TThostFtdcOrderPriceTypeType& ori, PriceType& des)
        {
            if (ori == THOST_FTDC_OPT_LimitPrice)
            {
                des = PriceTypeLimit;
            }
            else if (ori == THOST_FTDC_OPT_AnyPrice)
            {
                des = PriceTypeAny;
            }
        }
        inline void to_ctp_volume_condition(TThostFtdcVolumeConditionType& des, const VolumeCondition& ori)
        {
            if (ori == VolumeConditionAny)
            {
                des = THOST_FTDC_VC_AV;
            }
            else if (ori == VolumeConditionMin)
            {
                des = THOST_FTDC_VC_MV;
            }
            else if (ori == VolumeConditionAll)
            {
                des = THOST_FTDC_VC_CV;
            }
        }

        inline void from_ctp_volume_condition(const TThostFtdcVolumeConditionType& ori, VolumeCondition& des)
        {
            if (ori == THOST_FTDC_VC_AV)
            {
                des = VolumeConditionAny;
            }
            else if (ori == THOST_FTDC_VC_MV)
            {
                des = VolumeConditionMin;
            }
            else if (ori == THOST_FTDC_VC_CV)
            {
                des = VolumeConditionAll;
            }
        }

        inline void to_ctp_time_condition(TThostFtdcTimeConditionType& des, const TimeCondition& ori)
        {
            if (ori == TimeConditionGFD)
            {
                des = THOST_FTDC_TC_GFD;
            }
            else if (ori == TimeConditionIOC)
            {
                des = THOST_FTDC_TC_IOC;
            }
            else if (ori == TimeConditionGTC)
            {
                des = THOST_FTDC_TC_GTC;
            }
        }

        inline void from_ctp_time_condition(const TThostFtdcTimeConditionType& ori, TimeCondition& des)
        {
            if(ori == THOST_FTDC_TC_IOC)
            {
                des = TimeConditionIOC;
            }
            else if (ori == THOST_FTDC_TC_GFD)
            {
                des = TimeConditionGFD;
            }
            else if (ori == THOST_FTDC_TC_GTC)
            {
                des = TimeConditionGTC;
            }
        }

        inline void from_ctp_order_status(const TThostFtdcOrderStatusType& order_status, const TThostFtdcOrderSubmitStatusType& submit_status, OrderStatus& des)
        {
            switch(order_status)
            {
                case THOST_FTDC_OST_AllTraded: // 0 全部成交
                {
                    des = OrderStatusFilled;
                    break;
                }
                case THOST_FTDC_OST_PartTradedQueueing: // 1 部分成交，订单还在交易所撮合队列中
                {
                    des = OrderStatusPartialFilledActive;
                    break;
                }
                case THOST_FTDC_OST_PartTradedNotQueueing: // 2 部分成交，订单不再交易所撮合队列中
                {
                    des = OrderStatusPartialFilledNotActive;
                    break;
                }
                case THOST_FTDC_OST_NoTradeQueueing: // 3 未成交，订单还在交易所撮合队列中
                {
                    des = OrderStatusPending;
                    break;
                }
                case THOST_FTDC_OST_Canceled: // 5 已经撤单
                {
                    des = OrderStatusCancelled;
                    break;
                }
                case THOST_FTDC_OST_Unknown: // 未知
                {
                    if (submit_status == THOST_FTDC_OSS_InsertSubmitted)
                    {
                        des = OrderStatusSubmitted;
                    }
                    else if (submit_status == THOST_FTDC_OSS_Accepted)
                    {
                        des = OrderStatusPending;
                    }
                    else if (submit_status == THOST_FTDC_OSS_InsertRejected)
                    {
                        des = OrderStatusError;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        inline void from_ctp_time(const char* date, const char* update_time, int millisec, int64_t& nano_sec)
        {
            nano_sec = kungfu::yijinjing::parseTime((std::string(date) + "-" + std::string(update_time)).c_str(), "%Y%m%d-%H:%M:%S");
            nano_sec += millisec * kungfu::yijinjing::NANOSECONDS_PER_MILLISECOND;
        }

        inline void from_ctp_datetime(const TThostFtdcDateType date, const TThostFtdcTimeType time, int64_t& n_secs)
        {
            n_secs = kungfu::yijinjing::parseTime((std::string(date) + "-" + std::string(time)).c_str(), "%Y%m%d-%H:%M:%S");
        }

        inline void to_ctp(CThostFtdcDepthMarketDataField& des, const Quote& ori)
        {
            //TODO
        }

        inline void from_ctp(const CThostFtdcDepthMarketDataField& ori, Quote& des)
        {
            strcpy(des.source_id, SOURCE_CTP);
            strcpy(des.trading_day, ori.TradingDay);
            strcpy(des.instrument_id, ori.InstrumentID);
            strcpy(des.exchange_id, ori.ExchangeID);
            from_ctp_time(ori.ActionDay, ori.UpdateTime, ori.UpdateMillisec, des.data_time);
            des.last_price = ori.LastPrice;
            des.pre_settlement_price = ori.PreSettlementPrice;
            des.pre_close_price = ori.PreClosePrice;
            des.pre_open_interest = ori.PreOpenInterest;
            des.open_price = ori.OpenPrice;
            des.high_price = ori.HighestPrice;
            des.low_price = ori.LowestPrice;
            des.volume = ori.Volume;
            des.turnover = ori.Turnover;
            des.open_interest = ori.OpenInterest;
            des.close_price = ori.ClosePrice;
            des.settlement_price = is_too_large(ori.SettlementPrice) ? 0.0 : ori.SettlementPrice;
            des.upper_limit_price = ori.UpperLimitPrice;
            des.lower_limit_price = ori.LowerLimitPrice;

            des.bid_price[0] = ori.BidPrice1;
            des.bid_price[1] = ori.BidPrice2;
            des.bid_price[2] = ori.BidPrice3;
            des.bid_price[3] = ori.BidPrice4;
            des.bid_price[4] = ori.BidPrice5;

            des.ask_price[0] = ori.AskPrice1;
            des.ask_price[1] = ori.AskPrice2;
            des.ask_price[2] = ori.AskPrice3;
            des.ask_price[3] = ori.AskPrice4;
            des.ask_price[4] = ori.AskPrice5;

            des.bid_volume[0] = ori.BidVolume1;
            des.bid_volume[1] = ori.BidVolume2;
            des.bid_volume[2] = ori.BidVolume3;
            des.bid_volume[3] = ori.BidVolume4;
            des.bid_volume[4] = ori.BidVolume5;

            des.ask_volume[0] = ori.AskVolume1;
            des.ask_volume[1] = ori.AskVolume2;
            des.ask_volume[2] = ori.AskVolume3;
            des.ask_volume[3] = ori.AskVolume4;
            des.ask_volume[4] = ori.AskVolume5;

        }

        inline void to_ctp(CThostFtdcInputOrderField& des, const OrderInput& ori)
        {
            strcpy(des.InvestorID, ori.account_id);
            strcpy(des.InstrumentID, ori.instrument_id);
            #ifdef __linux__
                strcpy(des.ExchangeID, ori.exchange_id);
            #endif
            to_ctp_direction(des.Direction, ori.side);
            to_ctp_comb_offset(des.CombOffsetFlag, ori.offset);
            des.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
            des.VolumeTotalOriginal = ori.volume;
            des.ContingentCondition = THOST_FTDC_CC_Immediately;
            to_ctp_volume_condition(des.VolumeCondition, ori.volume_condition);
            to_ctp_time_condition(des.TimeCondition, ori.time_condition);
            des.MinVolume = 1;
            des.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
            des.IsAutoSuspend = 0;
            des.UserForceClose = 0;
            to_ctp_price_type(des.OrderPriceType, ori.price_type);
            des.LimitPrice = ori.limit_price;
        }

        inline void from_ctp(const CThostFtdcInputOrderField& ori, OrderInput& des)
        {
            strcpy(des.instrument_id, ori.InstrumentID);
#ifdef __linux__
            strcpy(des.exchange_id, ori.ExchangeID);
#else
            strcpy(des.exchange_id, get_exchange_id_from_future_instrument_id(ori.InstrumentID).c_str());
#endif
            strcpy(des.account_id, ori.InvestorID);
            from_ctp_direction(ori.Direction, des.side);
            from_ctp_comb_offset(ori.CombOffsetFlag, des.offset);
            des.volume = ori.VolumeTotalOriginal;
            from_ctp_volume_condition(ori.VolumeCondition, des.volume_condition);
            from_ctp_time_condition(ori.TimeCondition, des.time_condition);
            from_ctp_price_type(ori.OrderPriceType, des.price_type);
            des.limit_price = ori.LimitPrice;
        }

        inline void from_ctp(const CThostFtdcOrderField& ori, Order& des)
        {
            strcpy(des.instrument_id, ori.InstrumentID);
            strcpy(des.exchange_id, ori.ExchangeID);
            strcpy(des.account_id, ori.InvestorID);
            des.instrument_type = InstrumentTypeFuture;
            from_ctp_comb_offset(ori.CombOffsetFlag, des.offset);
            from_ctp_order_status(ori.OrderStatus, ori.OrderSubmitStatus, des.status);
            from_ctp_direction(ori.Direction, des.side);
            from_ctp_volume_condition(ori.VolumeCondition, des.volume_condition);
            from_ctp_time_condition(ori.TimeCondition, des.time_condition);
            des.limit_price = ori.LimitPrice;
            from_ctp_price_type(ori.OrderPriceType, des.price_type);
            strcpy(des.trading_day, ori.TradingDay);
            des.volume = ori.VolumeTotalOriginal;
            des.volume_left = ori.VolumeTotal;
            des.volume_traded = ori.VolumeTraded;
        }

        inline void from_ctp(const CThostFtdcTradeField& ori, Trade& des)
        {
            strcpy(des.instrument_id, ori.InstrumentID);
            strcpy(des.exchange_id, ori.ExchangeID);
            strcpy(des.account_id, ori.InvestorID);
            des.instrument_type = InstrumentTypeFuture;
            des.price = ori.Price;
            des.volume = ori.Volume;
            from_ctp_offset(ori.OffsetFlag, des.offset);
            from_ctp_direction(ori.Direction, des.side);
            from_ctp_datetime(ori.TradeDate, ori.TradeTime, des.trade_time);
        }

        inline void from_ctp(const CThostFtdcInvestorPositionField& ori, Position& des)
        {
            //strcpy(des.trading_day, ori.TradingDay);
            strcpy(des.instrument_id, ori.InstrumentID);
            des.instrument_type = InstrumentTypeFuture;
            strcpy(des.account_id, ori.InvestorID);
            strcpy(des.exchange_id, get_exchange_id_from_future_instrument_id(ori.InstrumentID).c_str());
            des.volume = ori.Position;
            des.yesterday_volume = ori.Position - ori.TodayPosition;

            from_ctp_pos_direction(ori.PosiDirection, des.direction);
            if (des.direction == DirectionLong)
            {
                des.frozen_total = ori.LongFrozen;
            }
            else if(des.direction == DirectionShort)
            {
                des.frozen_total = ori.ShortFrozen;
            }

            int multiplier = InstrumentManager::get_instrument_manager()->get_future_multiplier(des.instrument_id, des.exchange_id);
            des.cost_price = ori.PositionCost / multiplier / des.volume;

            des.pre_settlement_price = ori.PreSettlementPrice;
            des.settlement_price = is_too_large(ori.SettlementPrice) ? 0.0 : ori.SettlementPrice;

            des.margin = ori.UseMargin;

            des.realized_pnl = ori.CloseProfit;
            des.unrealized_pnl = ori.PositionProfit;
            des.position_pnl = des.realized_pnl + des.unrealized_pnl;

        }

        inline void from_ctp(const CThostFtdcInstrumentField& ori, FutureInstrument& des)
        {
            strcpy(des.instrument_id, ori.InstrumentID);
            strcpy(des.exchange_id, ori.ExchangeID);
            des.instrument_type = InstrumentTypeFuture;
            strcpy(des.product_id, gbk2utf8(ori.ProductID).c_str());
            des.delivery_year = ori.DeliveryYear;
            des.delivery_month = ori.DeliveryMonth;
            des.contract_multiplier = ori.VolumeMultiple;
            des.is_trading = ori.IsTrading;
            strcpy(des.create_date, ori.CreateDate);
            strcpy(des.expire_date, ori.ExpireDate);
            strcpy(des.open_date, ori.OpenDate);
            des.long_margin_ratio = ori.LongMarginRatio;
            des.short_margin_ratio = ori.ShortMarginRatio;
            des.price_tick = ori.PriceTick;
        }

        inline void from_ctp(const CThostFtdcInstrumentCommissionRateField& ori, InstrumentCommissionRate& des)
        {
            strcpy(des.instrument_id, ori.InstrumentID);
            if (! is_zero(ori.OpenRatioByMoney))
            {
                des.open_ratio = ori.OpenRatioByMoney;
                des.close_ratio = ori.CloseRatioByMoney;
                des.close_today_ratio = ori.CloseTodayRatioByMoney;
                des.mode = CommissionRateModeByAmount;
            }
            else if (! is_zero(ori.OpenRatioByVolume))
            {
                des.open_ratio = ori.OpenRatioByVolume;
                des.close_ratio = ori.CloseRatioByVolume;
                des.close_today_ratio = ori.CloseTodayRatioByVolume;
                des.mode = CommissionRateModeByVolume;
            }
        }

        inline void from_ctp(const CThostFtdcTradingAccountField& ori, AccountInfo& des)
        {
            strcpy(des.account_id, ori.AccountID);
            strcpy(des.broker_id, ori.BrokerID);
            strcpy(des.trading_day, ori.TradingDay);
            des.account_type = AccountTypeFuture;

            des.avail = ori.Available;

            des.position_pnl = ori.PositionProfit;
            des.close_pnl = ori.CloseProfit;

            des.margin = ori.CurrMargin;
            //des.fee = ori.Commission;

            des.frozen_margin = ori.FrozenMargin;
            des.frozen_fee = ori.FrozenCommission;
            des.frozen_cash = ori.FrozenCash;
        }

        inline void from_ctp(const CThostFtdcInvestorPositionDetailField& ori, Position& des)
        {
            //strcpy(des.trading_day, ori.TradingDay);
            strcpy(des.open_date, ori.OpenDate);
            strcpy(des.instrument_id, ori.InstrumentID);
            des.instrument_type = InstrumentTypeFuture;
            strcpy(des.exchange_id, ori.ExchangeID);
            strcpy(des.account_id, ori.InvestorID);
            des.direction = ori.Direction == THOST_FTDC_D_Buy ? DirectionLong : DirectionShort;
            des.volume = ori.Volume;
            des.open_price = ori.OpenPrice;
        }
    }
}
#endif //KUNGFU_DATAPARSER_H
