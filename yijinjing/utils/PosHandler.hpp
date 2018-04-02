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
 * Handle position automated update (replace PosMap)
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#ifndef YIJINJING_POSHANDLER_H
#define YIJINJING_POSHANDLER_H

#include "YJJ_DECLARE.h"
#include "json.hpp"
#include "KfLog.h"
#include "FeeHandler.hpp"
#include "TypeConvert.hpp"
#include "longfist/LFConstants.h"
#include "longfist/LFDataStruct.h"

using json = nlohmann::json;

YJJ_NAMESPACE_START

#define PH_EFFECTIVE_KEY    "ok"
#define PH_POSITIONS_KEY    "Pos"
#define PH_POS_COSTS_KEY    "Cost"
#define PH_TD_SOURCE_KEY    "Source"
#define PH_FEE_SETUP_KEY    "FeeSetup"

#define TOTAL_INDEX         0
#define AVAIL_INDEX         1 /**< available position */
#define YESTD_INDEX         1 /**< yesterday position (equivalent to avail_pos) */
#define POS_NUM_PER_DIR     2 /**< position number per direction */
#define DIRECTION_NUM       3 /**< Net, Long, Short */

#define PH_BALANCE_INDEX    0
#define PH_POS_FEE_INDEX    1
#define COST_NUM_PER_DIR    2

/** pos will be updated to long */
typedef int VOLUME_DATA_TYPE;

/**
 * easy marco for pos array index calculation
 * @param: dir (char) LF_CHAR_Net / LF_CHAR_Long / LF_CHAR_Short
 * @return: index in array
 */
#define POS_ARRAY_IDX(dir, off)  (POS_NUM_PER_DIR * (dir - '1') + off)
#define COST_ARRAY_IDX(dir, off)  POS_ARRAY_IDX(dir, off)

FORWARD_DECLARE_PTR(PosHandler);

class PosHandler
{
protected:
    json positions;
    json costs;
    bool is_poisoned;
    short source;
    FeeHandlerPtr fee_handler;
    bool is_stock_flag;

public:
    PosHandler(short source): is_poisoned(false), source(source)
    {
        fee_handler = FeeHandlerPtr(new FeeHandler());
        is_stock_flag = is_stock(source);
    };

    void init(const string& js_str)
    {
        json _js = json::parse(js_str);
        init_js(_js);
    }

    void init_js(const json& js)
    {
        is_poisoned = !js[PH_EFFECTIVE_KEY];
        positions = js[PH_POSITIONS_KEY];
        source = js[PH_TD_SOURCE_KEY];
        if (js.find(PH_POS_COSTS_KEY) != js.end())
        {
            costs = js[PH_POS_COSTS_KEY];
        }
        if (js.find(PH_FEE_SETUP_KEY) != js.end())
        {
            setup_fee(js[PH_FEE_SETUP_KEY]);
        }
    }

    void setup_fee(const json& js)
    {
        fee_handler->init(js);
    }

    void set_fee(FeeHandlerPtr fee)
    {
        fee_handler = fee;
    }

    static PosHandlerPtr create(short source);
    static PosHandlerPtr create(short source, const string& js_str);
    static PosHandlerPtr create(short source, json js);

    inline string to_string() const
    {
        json _js = to_json();
        return _js.dump();
    }

    json to_json() const
    {
        json _js;
        _js[PH_POSITIONS_KEY] = positions;
        _js[PH_EFFECTIVE_KEY] = !is_poisoned;
        _js[PH_TD_SOURCE_KEY] = source;
        _js[PH_POS_COSTS_KEY] = costs;
        _js[PH_FEE_SETUP_KEY] = fee_handler->to_json();
        return _js;
    }

    FeeHandlerPtr get_fee_handler() const
    {
        return fee_handler;
    }

    inline bool poisoned() const
    {
        return is_poisoned;
    }

    vector<string> get_tickers() const
    {
        vector<string> tickers;
        for (json::const_iterator iter = positions.begin(); iter != positions.end(); ++iter)
            tickers.push_back(iter.key());
        return tickers;
    }

    boost::python::list get_py_tickers() const
    {
        boost::python::list tickers;
        for (json::const_iterator iter = positions.begin(); iter != positions.end(); ++iter)
            tickers.append((string)iter.key());
        return tickers;
    }

    void print(KfLogPtr logger) const
    {
        for (string& ticker: get_tickers())
        {
            KF_LOG_INFO_FMT(logger, "[pos] (ticker)%s net:(t)%i(a)%i(y)%i long:(t)%i(a)%i(y)%i short:(t)%i(a)%i(y)%i",
                            ticker.c_str(), get_net_total(ticker), get_net_avail(ticker), get_net_yestd(ticker),
                            get_long_total(ticker), get_long_avail(ticker), get_long_yestd(ticker),
                            get_short_total(ticker), get_short_avail(ticker), get_short_yestd(ticker)
            );
        }
    }

    void switch_day()
    {
        for (json::iterator iter = positions.begin(); iter != positions.end(); ++iter)
        {
            json& pos_array = iter.value();
            pos_array[POS_ARRAY_IDX(LF_CHAR_Net, YESTD_INDEX)] = pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)];
            pos_array[POS_ARRAY_IDX(LF_CHAR_Long, YESTD_INDEX)] = pos_array[POS_ARRAY_IDX(LF_CHAR_Long, TOTAL_INDEX)];
            pos_array[POS_ARRAY_IDX(LF_CHAR_Short, YESTD_INDEX)] = pos_array[POS_ARRAY_IDX(LF_CHAR_Short, TOTAL_INDEX)];
        }
    }

    static inline bool is_stock(short source)
    {
        return source == SOURCE_XTP;
    }

    inline bool update(const string& ticker, VOLUME_DATA_TYPE volume, LfDirectionType direction, LfOffsetFlagType offset)
    {
        if (is_stock_flag)
            stock_update(ticker, volume, 0, direction, offset);
        else
            future_update(ticker, volume, 0, direction, offset);
        return !is_poisoned;
    }

    bool update_py(const string& ticker, VOLUME_DATA_TYPE volume, const string& direction, const string& trade_off)
    {
        return update(ticker, volume, direction[0], trade_off[0]);
    }

    inline bool update(const LFRtnTradeField* rtn_trade)
    {
        if (is_stock_flag)
            stock_update(rtn_trade->InstrumentID, rtn_trade->Volume, rtn_trade->Price, rtn_trade->Direction, rtn_trade->OffsetFlag);
        else
            future_update(rtn_trade->InstrumentID, rtn_trade->Volume, rtn_trade->Price, rtn_trade->Direction, rtn_trade->OffsetFlag);
        return !is_poisoned;
    }

    inline json& get_pos_array(const string& ticker)
    {
        if (positions.find(ticker) == positions.end())
        {
            json pos_array = json::array();
            for (size_t i = 0; i < DIRECTION_NUM * POS_NUM_PER_DIR; i++)
            {
                pos_array.push_back((VOLUME_DATA_TYPE)0);
            }
            positions[ticker] = pos_array;
        }
        return positions[ticker];
    }

    inline json& get_cost_array(const string& ticker)
    {
        if (costs.find(ticker) == costs.end())
        {
            json pos_array = json::array();
            for (size_t i = 0; i < DIRECTION_NUM * COST_NUM_PER_DIR; i++)
            {
                pos_array.push_back((double)0);
            }
            costs[ticker] = pos_array;
        }
        return costs[ticker];
    }

    inline void add_pos(const string& ticker, LfPosiDirectionType dir, VOLUME_DATA_TYPE tot, VOLUME_DATA_TYPE yd, double balance, double fee=0)
    {
        // add position info
        json& pos_array = get_pos_array(ticker);
        pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)] =
                pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() + tot;
        pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)] =
                pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)].get<VOLUME_DATA_TYPE>() + yd;
        // add cost info
        json& cost_array = get_cost_array(ticker);
        cost_array[COST_ARRAY_IDX(dir, PH_BALANCE_INDEX)] =
                cost_array[COST_ARRAY_IDX(dir, PH_BALANCE_INDEX)].get<double>() + balance;
        cost_array[COST_ARRAY_IDX(dir, PH_POS_FEE_INDEX)] =
                cost_array[COST_ARRAY_IDX(dir, PH_POS_FEE_INDEX)].get<double>() + fee;
    }

    void add_pos_py(const string& ticker, const string& dir, VOLUME_DATA_TYPE tot, VOLUME_DATA_TYPE yd, double balance, double fee)
    {
        add_pos(ticker, dir[0], tot, yd, balance, fee);
    }

    inline void set_pos(const string& ticker, LfPosiDirectionType dir, VOLUME_DATA_TYPE tot, VOLUME_DATA_TYPE yd, double balance, double fee)
    {
        // set position info
        json& pos_array = get_pos_array(ticker);
        pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)] = tot;
        pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)] = yd;
        // set cost info
        json& cost_array = get_cost_array(ticker);
        cost_array[COST_ARRAY_IDX(dir, PH_BALANCE_INDEX)] = balance;
        cost_array[COST_ARRAY_IDX(dir, PH_POS_FEE_INDEX)] = fee;
    }

    void set_pos_py(const string& ticker, const string& dir, VOLUME_DATA_TYPE tot, VOLUME_DATA_TYPE yd, double balance, double fee)
    {
        set_pos(ticker, dir[0], tot, yd, balance, fee);
    }

    inline double get_holding_value(const string& ticker, double last_price) const
    {
        int multiplier = fee_handler->get_contract_multiplier(ticker, is_stock_flag);
        if (positions.find(ticker) == positions.end())
            return 0;
        const json& pos_array = positions[ticker];

        VOLUME_DATA_TYPE net_pos = (is_stock_flag)
                                   ? pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>()
                                   : pos_array[POS_ARRAY_IDX(LF_CHAR_Long, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>()
                                     - pos_array[POS_ARRAY_IDX(LF_CHAR_Short, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>();

        return last_price * multiplier * net_pos;
    }

    inline double get_holding_balance(const string& ticker) const
    {
        if (costs.find(ticker) == costs.end())
            return 0;
        const json &cost_array = costs[ticker];

        double net_balance = (is_stock_flag)
                             ? cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_BALANCE_INDEX)].get<double>()
                             : cost_array[COST_ARRAY_IDX(LF_CHAR_Long, PH_BALANCE_INDEX)].get<double>()
                               - cost_array[COST_ARRAY_IDX(LF_CHAR_Short, PH_BALANCE_INDEX)].get<double>();

        return net_balance;
    }

    inline double get_holding_fee(const string& ticker) const
    {
        if (costs.find(ticker) == costs.end())
            return 0;
        const json& cost_array = costs[ticker];

        double net_fee = (is_stock_flag)
                         ? cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_POS_FEE_INDEX)].get<double>()
                         : cost_array[COST_ARRAY_IDX(LF_CHAR_Long, PH_POS_FEE_INDEX)].get<double>()
                           + cost_array[COST_ARRAY_IDX(LF_CHAR_Short, PH_POS_FEE_INDEX)].get<double>();

        return net_fee;
    }

    /***********************/
    /* EASY WAY TO GET POS */
    /***********************/
    inline VOLUME_DATA_TYPE get_pos_total(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, TOTAL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_pos_avail(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, AVAIL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_pos_yestd(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, YESTD_INDEX);
    }

    inline VOLUME_DATA_TYPE get_net_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, TOTAL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_net_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, AVAIL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_net_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, YESTD_INDEX);
    }

    inline VOLUME_DATA_TYPE get_long_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, TOTAL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_long_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, AVAIL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_long_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, YESTD_INDEX);
    }

    inline VOLUME_DATA_TYPE get_short_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, TOTAL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_short_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, AVAIL_INDEX);
    }

    inline VOLUME_DATA_TYPE get_short_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, YESTD_INDEX);
    }

    inline double get_fee(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_cost(ticker, dir, PH_POS_FEE_INDEX);
    }

    inline double get_balance(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_cost(ticker, dir, PH_BALANCE_INDEX);
    }

    inline double get_net_fee(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Net, PH_POS_FEE_INDEX);
    }

    inline double get_net_balance(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Net, PH_BALANCE_INDEX);
    }

    inline double get_long_fee(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Long, PH_POS_FEE_INDEX);
    }

    inline double get_long_balance(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Long, PH_BALANCE_INDEX);
    }

    inline double get_short_fee(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Short, PH_POS_FEE_INDEX);
    }

    inline double get_short_balance(const string& ticker) const
    {
        return get_cost(ticker, LF_CHAR_Short, PH_BALANCE_INDEX);
    }

protected:

    inline VOLUME_DATA_TYPE get_pos(const string& ticker, LfPosiDirectionType dir, short off) const
    {
        if (positions.find(ticker) == positions.end())
            return 0;
        else
        {
            return positions[ticker][POS_ARRAY_IDX(dir, off)];
        }
    }

    inline double get_cost(const string& ticker, LfPosiDirectionType dir, short off) const
    {
        if (costs.find(ticker) == costs.end())
            return 0;
        else
        {
            return costs[ticker][COST_ARRAY_IDX(dir, off)];
        }
    }

    inline void stock_update(const string& ticker, VOLUME_DATA_TYPE volume, double price, LfDirectionType direction, LfOffsetFlagType offset)
    {
        json& pos_array = get_pos_array(ticker);
        json& cost_array = get_cost_array(ticker);
        switch(direction)
        {
            case LF_CHAR_Buy:
            {
                pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)] =
                        pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() + volume;
                break;
            }
            case LF_CHAR_Sell:
            {
                pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)] =
                        pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                VOLUME_DATA_TYPE avail = pos_array[POS_ARRAY_IDX(LF_CHAR_Net, AVAIL_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                if (avail < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(LF_CHAR_Net, AVAIL_INDEX)] = avail;
                break;
            }
            default:
            {
                is_poisoned = true;
            }
        }

        cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_POS_FEE_INDEX)]
                = cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_POS_FEE_INDEX)].get<double>()
                  + fee_handler->get_fee(ticker, volume, price, direction, offset, true, false);

        cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_BALANCE_INDEX)]
                = cost_array[COST_ARRAY_IDX(LF_CHAR_Net, PH_BALANCE_INDEX)].get<double>()
                  + price * volume * ((direction == LF_CHAR_Buy) ? 1: -1);
    }

    inline void future_update(const string& ticker, VOLUME_DATA_TYPE volume, double price, LfDirectionType direction, LfOffsetFlagType offset)
    {
        json& pos_array = get_pos_array(ticker);
        json& cost_array = get_cost_array(ticker);
        bool close_today = false;
        LfPosiDirectionType position_dir = LF_CHAR_Net;
        switch (offset)
        {
            case LF_CHAR_Open:
            {
                position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Long : LF_CHAR_Short;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] =
                        pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() + volume;
                break;
            }
            case LF_CHAR_Close:
            case LF_CHAR_ForceClose:
            case LF_CHAR_ForceOff:
            case LF_CHAR_LocalForceClose:
            {
                // first open first close, for all four future exchange in China
                position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                // if there is yesterday position, minus
                VOLUME_DATA_TYPE yd = pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                if (yd >= 0)
                    pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] = yd;
                else
                    close_today = true;
                // then tot position needs to be revised.
                VOLUME_DATA_TYPE tot = pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                if (tot < pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] || tot < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] = tot;
                break;
            }
            case LF_CHAR_CloseToday:
            {
                position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                VOLUME_DATA_TYPE tot = pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                if (tot < pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] || tot < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] = tot;
                close_today = true;
                break;
            }
            case LF_CHAR_CloseYesterday:
            {
                position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] =
                        pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                VOLUME_DATA_TYPE yd = pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)].get<VOLUME_DATA_TYPE>() - volume;
                if (yd < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] = yd;
                break;
            }
            default:
            {
                is_poisoned = true;
            }
        }

        int multiplier = fee_handler->get_contract_multiplier(ticker, is_stock_flag);

        cost_array[COST_ARRAY_IDX(position_dir, PH_POS_FEE_INDEX)]
                = cost_array[COST_ARRAY_IDX(position_dir, PH_POS_FEE_INDEX)].get<double>()
                  + fee_handler->get_fee(ticker, volume, price, direction, offset, false, close_today);

        cost_array[COST_ARRAY_IDX(position_dir, PH_BALANCE_INDEX)]
                = cost_array[COST_ARRAY_IDX(position_dir, PH_BALANCE_INDEX)].get<double>()
                  + price * volume * multiplier * ((offset == LF_CHAR_Open) ? 1: -1);
    }
};


inline PosHandlerPtr PosHandler::create(short source)
{
    PosHandlerPtr res = PosHandlerPtr(new PosHandler(source));
    return res;
}

inline PosHandlerPtr PosHandler::create(short source, const string& js_str)
{
    PosHandlerPtr res = PosHandlerPtr(new PosHandler(source));
    res->init(js_str);
    return res;
}

inline PosHandlerPtr PosHandler::create(short source, json js)
{
    PosHandlerPtr res = PosHandlerPtr(new PosHandler(source));
    res->init_js(js);
    return res;
}

YJJ_NAMESPACE_END

#endif