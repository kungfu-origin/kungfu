/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Handle all transaction fee (tax and fee together)
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Dec, 2017
 */

#ifndef YIJINJING_FEEHANDLER_H
#define YIJINJING_FEEHANDLER_H

#include "YJJ_DECLARE.h"
#include "json.hpp"
#include "longfist/LFDataStruct.h"
#include <algorithm>

using json = nlohmann::json;

YJJ_NAMESPACE_START

#define CONTRACT_MULTIPLIER_KEY "ctr_multi"
#define FEE_MULTIPLIER_KEY      "fee_multi"

#define STOCK_BUY_MULTI_IDX     0
#define STOCK_SELL_MULTI_IDX    1
#define FUTURE_OPEN_MULTI_IDX   0
#define FUTURE_CLOSE_TODAY_MULTI_IDX   1
#define FUTURE_CLOSE_YESTD_MULTI_IDX   2

struct FeeDetail
{
    /** here is an exotic usage:
     *      use STOCK / FUTURE index for multiplier,
     *      at least [0] and [1] is effective, [2] will be useful
     */
    double multiplier[3];
    int    contract_multiplier;
    bool   using_volume;
    double min_fee;
    FeeDetail(): contract_multiplier(1), using_volume(true), min_fee(0)
    {
        for (size_t i = 0; i < 3; i++)
            multiplier[i] = 0;
    }
};

inline void from_json(const json& j, FeeDetail& fee)
{
    const json& j_multi = j[FEE_MULTIPLIER_KEY];
    if (j_multi.is_array())
    {
        size_t i = 0;
        for (; i < j_multi.size(); i++)
            fee.multiplier[i] = j_multi[i].get<double>();
        for (; i < 3; i++)
            fee.multiplier[i] = fee.multiplier[i - 1];
    }
    else
    {
        fee.multiplier[0] = j_multi.get<double>();
        fee.multiplier[1] = j_multi.get<double>();
        fee.multiplier[2] = j_multi.get<double>();
    }
    fee.using_volume = (j["type"].get<string>() == "volume");
    fee.min_fee = j["min_fee"].get<double>();
    if (j.find(CONTRACT_MULTIPLIER_KEY) != j.end())
    {
        fee.contract_multiplier = j[CONTRACT_MULTIPLIER_KEY].get<int>();
    }
}

inline void to_json(json& j, const FeeDetail& fee)
{
    j["type"] = fee.using_volume ? "volume": "amount";
    j["min_fee"] = fee.min_fee;
    size_t idx = 2;
    while (idx > 0 && fee.multiplier[idx] == fee.multiplier[idx - 1])
        idx --;
    json j_multi;
    if (idx == 0)
    {
        j_multi = fee.multiplier[0];
    }
    else
    {
        j_multi = json::array();
        for (size_t i = 0; i <= idx; i++)
        {
            j_multi.push_back(fee.multiplier[i]);
        }
    }
    j[FEE_MULTIPLIER_KEY] = j_multi;
    j[CONTRACT_MULTIPLIER_KEY] = fee.contract_multiplier;
}

class FeeHandler
{
protected:
    /** stock fee */
    FeeDetail stock;
    /** general future fee */
    FeeDetail future;
    /** from product name to fee details */
    map<string, FeeDetail> future_exotic;

public:

    FeeHandler() {};

    FeeHandler(const string& js_str)
    {
        json _js = json::parse(js_str);
        init(_js);
    }

    inline void init(const json& js)
    {
        stock = js["stock"].get<FeeDetail>();
        future = js["future"].get<FeeDetail>();
        json exotics = js["future_exotic"];
        if (exotics.is_object())
        {
            for (json::const_iterator iter = exotics.begin(); iter != exotics.end(); ++iter)
            {
                string ticker = iter.key();
                std::transform(ticker.begin(), ticker.end(), ticker.begin(), ::tolower);
                future_exotic[ticker] = iter.value().get<FeeDetail>();
            }
        }
    }

    FeeHandler(const json& js)
    {
        init(js);
    }

    inline json to_json() const
    {
        json js;
        js["stock"] = stock;
        js["future"] = future;
        json exotics;
        for (auto &iter: future_exotic)
            exotics[iter.first] = iter.second;
        js["future_exotic"] = exotics;
        return js;
    }

    /**
     * get commission and tax amount from trade
     * @param rtn_trade
     * @param is_stock
     * @param close_today: flag, only useful when get future fee with close offset
     * @return
     */
    inline double get_fee(const LFRtnTradeField* rtn_trade, bool is_stock, bool close_today=false) const
    {
        return get_fee(rtn_trade->InstrumentID, rtn_trade->Volume, rtn_trade->Price, rtn_trade->Direction, rtn_trade->OffsetFlag, is_stock, close_today);
    }

    inline int get_contract_multiplier(const string& ticker, bool is_stock)
    {
        if (is_stock)
            return 1;
        else
        {
            const FeeDetail* detail = get_future_fee_detail(ticker);
            return detail->contract_multiplier;
        }
    }

    inline const FeeDetail* get_future_fee_detail(const string& _ticker) const
    {
        const FeeDetail* detail = nullptr;
        if (future_exotic.size() > 0)
        {
            string ticker = _ticker;
            std::transform(ticker.begin(), ticker.end(), ticker.begin(), ::tolower);
            auto iter = future_exotic.find(ticker);
            if (iter == future_exotic.end())
            {
                size_t idx = 0;
                char c = ticker[idx];
                while ((c > '9' || c < '0') && idx < ticker.size() - 1)
                {
                    idx ++;
                    c = ticker[idx];
                }
                string product = ticker.substr(0, idx);
                auto iter_prod = future_exotic.find(product);
                if (iter_prod != future_exotic.end())
                {
                    detail = &iter_prod->second;
                }
            }
            else
            {
                detail = &iter->second;
            }
        }
        if (detail == nullptr)
        {
            detail = &future;
        }
        return detail;
    }

    inline double get_fee(const string& ticker, int volume, double price, LfDirectionType direction, LfOffsetFlagType offset, bool is_stock, bool close_today=false) const
    {
        if (is_stock)
        {
            double amount = price * volume;
            return amount * stock.multiplier[direction == LF_CHAR_Buy ? STOCK_BUY_MULTI_IDX : STOCK_SELL_MULTI_IDX];
        }
        else
        {
            const FeeDetail* detail = get_future_fee_detail(ticker);
            double base = volume;
            if (!detail->using_volume)
            {
                base *= price * detail->contract_multiplier;
            }
            switch (offset)
            {
                case LF_CHAR_Open:
                {
                    return base * detail->multiplier[FUTURE_OPEN_MULTI_IDX];
                }
                case LF_CHAR_Close:
                case LF_CHAR_ForceClose:
                case LF_CHAR_ForceOff:
                case LF_CHAR_LocalForceClose:
                {
                    return base * detail->multiplier[close_today ? FUTURE_CLOSE_TODAY_MULTI_IDX : FUTURE_CLOSE_YESTD_MULTI_IDX];
                }
                case LF_CHAR_CloseToday:
                {
                    return base * detail->multiplier[FUTURE_CLOSE_TODAY_MULTI_IDX];
                }
                case LF_CHAR_CloseYesterday:
                {
                    return base * detail->multiplier[FUTURE_CLOSE_YESTD_MULTI_IDX];
                }
                default:
                {
                    return 0;
                }
            }
        }
    }
};

DECLARE_PTR(FeeHandler);

YJJ_NAMESPACE_END

#endif