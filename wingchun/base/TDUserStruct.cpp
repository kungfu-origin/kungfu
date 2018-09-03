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
 * TDUserStruct: struct of Trade engine internal usage.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#include "TDUserStruct.h"
#include "PageUtil.h"
#include "Timer.h"
#include "longfist/LFConstants.h"
#include <sstream>

USING_WC_NAMESPACE
USING_YJJ_NAMESPACE

TDUserInfoHelper::TDUserInfoHelper(short source): source(source)
{
    address_book.clear();
}

TDUserInfoHelper::TDUserInfoHelper(short source, const string& user_name): TDUserInfoHelper(source)
{
    // force load read-only in this common constructor. only ITDEngine may modify
    load(user_name, false);
}

void TDUserInfoHelper::load(const string& user_name, bool need_write)
{
    if (address_book.find(user_name) == address_book.end())
    {
        std::stringstream ss;
        ss << TD_USER_INFO_FOLDER << user_name << "." << source;
        TDUserInfo* address = (TDUserInfo*)PageUtil::LoadPageBuffer(ss.str(), USER_INFO_SIZE, need_write, true);
        if (need_write)
        {
            address->status = TD_INFO_WRITING;
            address->start_time = getNanoTime();
            strncpy(address->name, user_name.c_str(), JOURNAL_SHORT_NAME_MAX_LENGTH);
        }
        address_book[user_name] = address;
    }
}

void TDUserInfoHelper::remove(const string& user_name)
{
    if (address_book.find(user_name) != address_book.end())
    {
        TDUserInfo* address = address_book[user_name];
        address->end_time = getNanoTime();
        address->status = TD_INFO_NORMAL;
        address_book.erase(user_name);
        PageUtil::ReleasePageBuffer(address, USER_INFO_SIZE, true);
    }
}


TDUserInfoHelper::~TDUserInfoHelper()
{
    for (auto iter: address_book)
    {
        TDUserInfo* address = iter.second;
        address->end_time = getNanoTime();
        address->status = TD_INFO_FORCE_QUIT;
        PageUtil::ReleasePageBuffer(address, USER_INFO_SIZE, true);
    }
}

void TDUserInfoHelper::switch_day()
{
    orders.clear();
}

inline bool order_is_existing(char status)
{
    return status != ORDER_INFO_RAW
           && status != LF_CHAR_Error
           && status != LF_CHAR_AllTraded
           && status != LF_CHAR_Canceled
           && status != LF_CHAR_PartTradedNotQueueing;
}

void TDUserInfoHelper::record_order(const string& user_name, int local_id, int order_id, const char* ticker)
{
    auto& order_info = orders[user_name][local_id];
    order_info.order_id = order_id;
    order_info.local_id = local_id;
    order_info.status = LF_CHAR_OrderInserted;
    order_info.ticker = ticker;
}

bool TDUserInfoHelper::get_order(const string& user_name, int order_id, int &local_id, char* ticker) const
{
    auto iter = orders.find(user_name);
    if (iter != orders.end())
    {
        for (auto o_iter = iter->second.begin(); o_iter != iter->second.end(); ++o_iter)
        {
            if (o_iter->second.order_id == order_id)
            {
                local_id = o_iter->second.local_id;
                strncpy(ticker, o_iter->second.ticker.c_str(), ORDER_INFO_TICKER_LIMIT);
                return true;
            }
        }
    }

    return false;
}

json TDUserInfoHelper::get_pos(const string& user_name) const
{
    json pos;
    auto info = get_user_info(user_name);
    if (info == nullptr || info->pos_str[0] != '{')
    {
        pos["ok"] = false;
    }
    else
    {
        pos = json::parse(info->pos_str);
        pos["ok"] = true;
    }
    pos["name"] = user_name;
    return pos;
}

void TDUserInfoHelper::set_pos(const string& user_name, const json& pos)
{
    json j_pos = pos;
    j_pos["nano"] = kungfu::yijinjing::getNanoTime();
    j_pos["name"] = user_name;
    /** if loaded, simply skip internally */
    load(user_name);
    TDUserInfo* info = address_book[user_name];
    strcpy(info->pos_str, j_pos.dump().c_str());
}

void TDUserInfoHelper::set_order_status(const string& user_name, int order_id, char status)
{
    auto& user_orders = orders[user_name];
    for (auto iter = user_orders.begin(); iter != user_orders.end(); ++iter)
    {
        if (iter->second.order_id == order_id)
        {
            iter->second.status = status;
        }
    }
}

vector<int> TDUserInfoHelper::get_existing_orders(const string& user_name) const
{
    vector<int> order_ids;
    auto iter = orders.find(user_name);
    if (iter != orders.end())
    {
        for (auto o_iter = iter->second.begin(); o_iter != iter->second.end(); ++o_iter)
        {
            if (order_is_existing(o_iter->second.status))
            {
                order_ids.push_back(o_iter->second.order_id);
            }
        }
    }

    return order_ids;
}

const TDUserInfo* TDUserInfoHelper::get_const_user_info(const string& user_name) const
{
    return get_user_info(user_name);
}

/***********************/
/** TDEngineInfoHelper */
/***********************/

TDEngineInfoHelper::TDEngineInfoHelper(short source, const string& name)
{
}

TDEngineInfoHelper::~TDEngineInfoHelper()
{
}

void TDEngineInfoHelper::switch_day()
{
    cleanup();
}

void TDEngineInfoHelper::cleanup()
{
    orders.clear();
}

void TDEngineInfoHelper::set_order_status(int local_id, char status)
{
    auto iter = orders.find(local_id);
    if (iter != orders.end())
    {
        iter->second.status = status;
    }
}

int TDEngineInfoHelper::get_order_id(int local_id)
{
    auto iter = orders.find(local_id);
    if (iter != orders.end())
    {
        return iter->second.order_id;
    }

    return -1;
}

void TDEngineInfoHelper::record_order(int local_id, int order_id)
{
    auto& order = orders[local_id];
    order.order_id = order_id;
    order.local_id = local_id;
    order.status = LF_CHAR_OrderInserted;
}