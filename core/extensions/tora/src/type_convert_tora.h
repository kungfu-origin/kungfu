//
// Created by PolarAir on 2019-06-18.
//

#ifndef KUNGFU_TYPE_CONVERT_TORA_H
#define KUNGFU_TYPE_CONVERT_TORA_H

#include "TORATstpUserApiStruct.h"
#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu
{
    namespace tora
    {
        inline void from_tora(TTORATstpExchangeIDType tora_exchange, char* exchange_id)
        {
            if (tora_exchange == TORA_TSTP_EXD_SSE)
            {
                strcpy(exchange_id, EXCHANGE_SSE);
            }
            else if (tora_exchange == TORA_TSTP_EXD_SZSE)
            {
                strcpy(exchange_id, EXCHANGE_SZE);
            }
            else
            {
                strcpy(exchange_id, "");
            }
        }

        inline void to_tora(const char* exchange_id, TTORATstpExchangeIDType& tora_exchange)
        {
            if (strcmp(exchange_id, EXCHANGE_SSE) == 0)
            {
                tora_exchange = TORA_TSTP_EXD_SSE;
            }
            else if (strcmp(exchange_id, EXCHANGE_SZE) == 0)
            {
                tora_exchange = TORA_TSTP_EXD_SZSE;
            }
            else
            {
                tora_exchange = TORA_TSTP_EXD_COMM;
            }
        }

        inline void from_tora(const CTORATstpMarketDataField& md, Quote& quote)
        {
            memset(&quote, 0, sizeof(Quote));
            strcpy(quote.source_id, SOURCE_TORA);
            strcpy(quote.trading_day, md.TradingDay);
            quote.data_time = yijinjing::time::strptime((std::string(md.TradingDay) + std::string(md.UpdateTime)).c_str(), "%Y%m%d%H:%M:%S")
                    + md.UpdateMillisec * yijinjing::time_unit::NANOSECONDS_PER_MILLISECOND;
            strcpy(quote.instrument_id, md.SecurityID);
            from_tora(md.ExchangeID, quote.exchange_id);
            if (is_reverse_repurchase(quote.instrument_id, quote.exchange_id))
            {
                quote.instrument_type = InstrumentTypeStock;
            }
            else
            {
                quote.instrument_type = InstrumentTypeBond;
            }
            quote.pre_close_price = md.PreClosePrice;
            quote.last_price = md.LastPrice;
            quote.volume = md.Volume;
            quote.turnover = md.Turnover;
            quote.open_price = md.OpenPrice;
            quote.high_price = md.HighestPrice;
            quote.low_price = md.LowestPrice;
            quote.upper_limit_price = md.UpperLimitPrice;
            quote.lower_limit_price = md.LowerLimitPrice;
            quote.close_price = md.ClosePrice;
            quote.bid_price[0] = md.BidPrice1;
            quote.ask_price[0] = md.AskPrice1;
            quote.bid_volume[0] = md.BidVolume1;
            quote.ask_volume[0] = md.AskVolume1;
            quote.bid_price[1] = md.BidPrice2;
            quote.ask_price[1] = md.AskPrice2;
            quote.bid_volume[1] = md.BidVolume2;
            quote.ask_volume[1] = md.AskVolume2;
            quote.bid_price[2] = md.BidPrice3;
            quote.ask_price[2] = md.AskPrice3;
            quote.bid_volume[2] = md.BidVolume3;
            quote.ask_volume[2] = md.AskVolume3;
            quote.bid_price[3] = md.BidPrice4;
            quote.ask_price[3] = md.AskPrice4;
            quote.bid_volume[3] = md.BidVolume4;
            quote.ask_volume[3] = md.AskVolume4;
            quote.bid_price[4] = md.BidPrice5;
            quote.ask_price[4] = md.AskPrice5;
            quote.bid_volume[4] = md.BidVolume5;
            quote.ask_volume[4] = md.AskVolume5;
        }
    }
}


#endif //KUNGFU_TYPE_CONVERT_TORA_H
