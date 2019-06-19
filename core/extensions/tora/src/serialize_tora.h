//
// Created by PolarAir on 2019-06-18.
//

#ifndef KUNGFU_SERIALIZE_TORA_H
#define KUNGFU_SERIALIZE_TORA_H

#include "TORATstpUserApiStruct.h"
#include <nlohmann/json.hpp>

namespace kungfu
{
    namespace tora
    {
        inline void to_json(nlohmann::json& j, const CTORATstpMarketDataField& md)
        {
            j["TradingDay"] = md.TradingDay;
            j["SecurityID"] = md.SecurityID;
            j["ExchangeID"] = md.ExchangeID;
            j["SecurityName"] = md.SecurityName;
            j["PreClosePrice"] = md.PreClosePrice;
            j["OpenPrice"] = md.OpenPrice;
            j["Volume"] = md.Volume;
            j["Turnover"] = md.Turnover;
            j["TradingCount"] = md.TradingCount;
            j["LastPrice"] = md.LastPrice;
            j["HighestPrice"] = md.HighestPrice;
            j["LowestPrice"] = md.LowestPrice;
            j["BidPrice1"] = md.BidPrice1;
            j["AskPrice1"] = md.AskPrice1;
            j["UpperLimitPrice"] = md.UpperLimitPrice;
            j["LowerLimitPrice"] = md.LowerLimitPrice;
            j["PERatio1"] = md.PERatio1;
            j["PERatio2"] = md.PERatio2;
            j["PriceUpDown1"] = md.PriceUpDown1;
            j["PriceUpDown2"] = md.PriceUpDown2;
            j["OpenInterest"] = md.OpenInterest;
            j["BidVolume1"] = md.BidVolume1;
            j["AskVolume1"] = md.AskVolume1;
            j["BidPrice2"] = md.BidPrice2;
            j["BidVolume2"] = md.BidVolume2;
            j["AskPrice2"] = md.AskPrice2;
            j["AskVolume2"] = md.AskVolume2;
            j["BidPrice3"] = md.BidPrice3;
            j["BidVolume3"] = md.BidVolume3;
            j["AskPrice3"] = md.AskPrice3;
            j["AskVolume3"] = md.AskVolume3;
            j["BidPrice4"] = md.BidPrice4;
            j["BidVolume4"] = md.BidVolume4;
            j["AskPrice4"] = md.AskPrice4;
            j["AskVolume4"] = md.AskVolume4;
            j["BidPrice5"] = md.BidPrice5;
            j["BidVolume5"] = md.BidVolume5;
            j["AskPrice5"] = md.AskPrice5;
            j["AskVolume5"] = md.AskVolume5;
            j["UpdateTime"] = md.UpdateTime;
            j["UpdateMillisec"] = md.UpdateMillisec;
            j["ClosePrice"] = md.ClosePrice;
            j["MDSecurityStat"] = md.MDSecurityStat;
            j["HWFlag"] = md.HWFlag;
        }

        template <typename T>
        std::string to_string(const T& ori)
        {
            nlohmann::json j;
            to_json(j, ori);
            return j.dump();
        }
    }
}

#endif //KUNGFU_SERIALIZE_TORA_H
