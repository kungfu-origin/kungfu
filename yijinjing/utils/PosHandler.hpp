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
#include "TypeConvert.hpp"
#include "longfist/LFConstants.h"
#include "longfist/LFDataStruct.h"

using json = nlohmann::json;

YJJ_NAMESPACE_START

#define TOTAL_INDEX         0
#define AVAIL_INDEX         1 /**< available position */
#define YESTD_INDEX         1 /**< yesterday position (equivalent to avail_pos) */
#define POS_NUM_PER_DIR     2 /**< position number per direction */
#define DIRECTION_NUM       3 /**< Net, Long, Short */

/**
 * easy marco for pos array index calculation
 * @param: dir (char) LF_CHAR_Net / LF_CHAR_Long / LF_CHAR_Short
 * @return: index in array
 */
#define POS_ARRAY_IDX(dir, off)  (POS_NUM_PER_DIR * (dir - '1') + off)

FORWARD_DECLARE_PTR(PosHandler);

class PosHandler
{
protected:
    json positions;
    bool is_poisoned;
    short source;
public:
    PosHandler(short source): is_poisoned(false), source(source) {};

    void init(const string& js_str)
    {
        json _js = json::parse(js_str);
        is_poisoned = !_js["ok"];
        positions = _js["Pos"];
        source = _js["Source"];
    }

    static PosHandlerPtr create(short source);
    static PosHandlerPtr create(short source, const string& js_str);

    inline string to_string() const
    {
        json _js = to_json();
        return _js.dump();
    }

    inline json to_json() const
    {
        json _js;
        _js["Pos"] = positions;
        _js["ok"] = !is_poisoned;
        _js["Source"] = source;
        return _js;
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

    inline void init_ticker(const string& ticker)
    {
        json pos_array = json::array();
        for (int i = 0; i < DIRECTION_NUM * POS_NUM_PER_DIR; i++)
            pos_array.push_back(0);
        positions[ticker] = pos_array;
    }

    inline bool update(const string& ticker, int volume, LfDirectionType direction, LfOffsetFlagType trade_off, LfOffsetFlagType order_off)
    {
        LfOffsetFlagType offset = judge_offset(source, trade_off, order_off);
        if (source == SOURCE_LTS || source == SOURCE_HUNDSUN || source == SOURCE_OES)
            return stock_update(ticker, volume, direction, offset);
        else
            return future_update(ticker, volume, direction, offset);
    }

    inline bool update(const LFRtnTradeField* rtn_trade)
    {
        return update(rtn_trade->InstrumentID, rtn_trade->Volume, rtn_trade->Direction, rtn_trade->OffsetFlag, rtn_trade->OffsetFlag);
    }

    bool update_py(const string& ticker, int volume, const string& direction, const string& trade_off, const string& order_off)
    {
        return update(ticker, volume, direction[0], trade_off[0], order_off[0]);
    }

    inline json& get_pos_array(const string& ticker)
    {
        if (positions.find(ticker) == positions.end())
        {
            init_ticker(ticker);
        }
        return positions[ticker];
    }

    inline void add_pos(const string& ticker, LfPosiDirectionType dir, int tot, int yd)
    {
        json& pos_array = get_pos_array(ticker);
        pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)] =
            pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)].get<int>() + tot;
        pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)] =
            pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)].get<int>() + yd;
    }

    void add_pos_py(const string& ticker, const string& dir, int tot, int yd)
    {
        add_pos(ticker, dir[0], tot, yd);
    }

    inline void set_pos(const string& ticker, LfPosiDirectionType dir, int tot, int yd)
    {
        json& pos_array = get_pos_array(ticker);
        pos_array[POS_ARRAY_IDX(dir, TOTAL_INDEX)] = tot;
        pos_array[POS_ARRAY_IDX(dir, YESTD_INDEX)] = yd;
    }

    void set_pos_py(const string& ticker, const string& dir, int tot, int yd)
    {
        set_pos(ticker, dir[0], tot, yd);
    }

    static inline LfOffsetFlagType judge_offset(short source, LfOffsetFlagType trade_offset, LfOffsetFlagType order_offset)
    {
        // issue 1: some api don't return proper OFFSET.
        // here, for CTP, closeYesterday and closeToday will only result in close here in RTN_TRADE
        // then we need close from order....
        // any other api has similar behavior, please let cjiang or qlu know....
        if (source == SOURCE_CTP)
            return order_offset;
        else
            return trade_offset;
    }
    /***********************/
    /* EASY WAY TO GET POS */
    /***********************/
    inline int get_pos_total(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, TOTAL_INDEX);
    }

    inline int get_pos_avail(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, AVAIL_INDEX);
    }

    inline int get_pos_yestd(const string& ticker, LfPosiDirectionType dir) const
    {
        return get_pos(ticker, dir, YESTD_INDEX);
    }

    inline int get_net_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, TOTAL_INDEX);
    }

    inline int get_net_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, AVAIL_INDEX);
    }

    inline int get_net_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Net, YESTD_INDEX);
    }

    inline int get_long_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, TOTAL_INDEX);
    }

    inline int get_long_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, AVAIL_INDEX);
    }

    inline int get_long_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Long, YESTD_INDEX);
    }

    inline int get_short_total(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, TOTAL_INDEX);
    }

    inline int get_short_avail(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, AVAIL_INDEX);
    }

    inline int get_short_yestd(const string& ticker) const
    {
        return get_pos(ticker, LF_CHAR_Short, YESTD_INDEX);
    }

protected:

    inline int get_pos(const string& ticker, LfPosiDirectionType dir, short off) const
    {
        if (positions.find(ticker) == positions.end())
            return 0;
        else
        {
            int index = POS_NUM_PER_DIR * (dir - '1') + off;
            return positions[ticker][index];
        }
    }

    inline bool stock_update(const string& ticker, int volume, LfDirectionType direction, LfOffsetFlagType offset)
    {
        json& pos_array = get_pos_array(ticker);

        switch(direction)
        {
            case LF_CHAR_Buy:
            {
                pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)] =
                    pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)].get<int>() - volume;
                break;
            }
            case LF_CHAR_Sell:
            {
                pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)] =
                    pos_array[POS_ARRAY_IDX(LF_CHAR_Net, TOTAL_INDEX)].get<int>() - volume;
                int avail = pos_array[POS_ARRAY_IDX(LF_CHAR_Net, AVAIL_INDEX)].get<int>() - volume;
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
        return !is_poisoned;
    }

    inline bool future_update(const string& ticker, int volume, LfDirectionType direction, LfOffsetFlagType offset)
    {
        json& pos_array = get_pos_array(ticker);

        switch (offset)
        {
            case LF_CHAR_Open:
            {
                LfPosiDirectionType position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Long : LF_CHAR_Short;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] =
                    pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<int>() + volume;
                break;
            }
            case LF_CHAR_Close:
            case LF_CHAR_ForceClose:
            case LF_CHAR_ForceOff:
            case LF_CHAR_LocalForceClose:
            {
                // first open first close, for all four future exchange in China
                LfPosiDirectionType position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                int tot = pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<int>() - volume;
                if (tot < pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] || tot < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] = tot;
                int yd = pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)].get<int>() - volume;
                if (yd >= 0)
                    pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] = yd;
                break;
            }
            case LF_CHAR_CloseToday:
            {
                LfPosiDirectionType position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                int tot = pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<int>() - volume;
                if (tot < pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)] || tot < 0)
                    is_poisoned = true;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] = tot;
                break;
            }
            case LF_CHAR_CloseYesterday:
            {
                LfPosiDirectionType position_dir = (direction == LF_CHAR_Buy) ? LF_CHAR_Short : LF_CHAR_Long;
                pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)] =
                    pos_array[POS_ARRAY_IDX(position_dir, TOTAL_INDEX)].get<int>() - volume;
                int yd = pos_array[POS_ARRAY_IDX(position_dir, YESTD_INDEX)].get<int>() - volume;
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
        return !is_poisoned;
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

YJJ_NAMESPACE_END

#endif