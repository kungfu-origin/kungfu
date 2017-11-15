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
        clean_up(address);
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
    for (auto iter: address_book)
    {
        clean_up(iter.second);
    }
}

void TDUserInfoHelper::clean_up(TDUserInfo* info)
{
    info->last_order_index = -1;
    memset(info->orders, '\0', AVAILABLE_ORDER_LIMIT * sizeof(TDOrderInfo));
}

TDOrderInfo* TDUserInfoHelper::locate_readable(const string& user_name, int order_id) const
{
    TDUserInfo* info = get_user_info(user_name);
    if (info == nullptr)
        return nullptr;
    int idx = order_id % AVAILABLE_ORDER_LIMIT;
    int count = 0;
    while (info->orders[idx].order_id != order_id && count < AVAILABLE_ORDER_LIMIT)
    {
        idx = (idx + 1) % AVAILABLE_ORDER_LIMIT;
        count++;
    }
    if (count == AVAILABLE_ORDER_LIMIT)
        return nullptr;
    else
        return &(info->orders[idx]);
}

inline bool order_is_existing(char status)
{
    return status != ORDER_INFO_RAW
           && status != LF_CHAR_Error
           && status != LF_CHAR_AllTraded
           && status != LF_CHAR_Canceled
           && status != LF_CHAR_PartTradedNotQueueing;
}

TDOrderInfo* TDUserInfoHelper::locate_writable(const string &user_name, int order_id)
{
    TDUserInfo* info = get_user_info(user_name);
    if (info == nullptr)
        return nullptr;
    int idx = order_id % AVAILABLE_ORDER_LIMIT;
    int count = 0;
    char status = info->orders[idx].status;
    while (order_is_existing(status) && count < AVAILABLE_ORDER_LIMIT)
    {
        idx = (idx + 1) % AVAILABLE_ORDER_LIMIT;
        status = info->orders[idx].status;
        count++;
    }
    if (count == AVAILABLE_ORDER_LIMIT)
        return nullptr;
    else
    {
        if (idx > info->last_order_index)
            info->last_order_index = idx;
        return &(info->orders[idx]);
    }
}

void TDUserInfoHelper::record_order(const string& user_name, int local_id, int order_id, const char* ticker)
{
    TDOrderInfo* order_info = locate_writable(user_name, order_id);
    if (order_info != nullptr)
    {
        order_info->order_id = order_id;
        order_info->local_id = local_id;
        order_info->status = LF_CHAR_OrderInserted;
        strncpy(order_info->ticker, ticker, ORDER_INFO_TICKER_LIMIT);
    }
}

bool TDUserInfoHelper::get_order(const string& user_name, int order_id, int &local_id, char* ticker) const
{
    TDOrderInfo* order_info = locate_readable(user_name, order_id);
    if (order_info != nullptr)
    {
        local_id = order_info->local_id;
        strncpy(ticker, order_info->ticker, ORDER_INFO_TICKER_LIMIT);
        return true;
    }
    else
    {
        return false;
    }
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
    TDOrderInfo* order_info = locate_readable(user_name, order_id);
    if (order_info != nullptr)
    {
        order_info->status = status;
    }
}

vector<int> TDUserInfoHelper::get_existing_orders(const string& user_name) const
{
    vector<int> order_ids;
    auto info = get_user_info(user_name);
    if (info != nullptr)
    {
        for (int idx = 0; idx <= info->last_order_index; idx++)
        {
            TDOrderInfo& order = info->orders[idx];
            if (order_is_existing(order.status))
                order_ids.push_back(order.order_id);
        }
    }
    return order_ids;
}

const TDUserInfo* TDUserInfoHelper::get_const_user_info(const string& user_name) const
{
    return get_user_info(user_name);
}

/** TDEngineInfoHelper */
TDEngineInfoHelper::TDEngineInfoHelper(short source, const string& name)
{
    std::stringstream ss;
    ss << TD_USER_INFO_FOLDER << name;
    info = (TDEngineInfo*)PageUtil::LoadPageBuffer(ss.str(), TD_ENGINE_INFO_SIZE, true, true);
    if (info->status != TD_INFO_NORMAL)
    {
        memset(info, '\0', TD_ENGINE_INFO_SIZE);
        info->version = TD_ENGINE_INFO_VERSION;
        info->source = source;
    }
    info->status = TD_INFO_WRITING;
    info->start_time = getNanoTime();
    info->end_time = -1;
    cleanup();
}

TDEngineInfoHelper::~TDEngineInfoHelper()
{
    info->end_time = getNanoTime();
    info->status = TD_INFO_NORMAL;
    PageUtil::ReleasePageBuffer(info, USER_INFO_SIZE, true);
}

TDBasicOrderInfo* TDEngineInfoHelper::locate_readable(int local_id) const
{
    int idx = local_id % TD_AVAILABLE_ORDER_LIMIT;
    int count = 0;
    while (info->orders[idx].local_id != local_id && count < TD_AVAILABLE_ORDER_LIMIT)
    {
        idx = (idx + 1) % TD_AVAILABLE_ORDER_LIMIT;
        count ++;
    }
    if (count == TD_AVAILABLE_ORDER_LIMIT)
        return nullptr;
    else
        return &(info->orders[idx]);
}

TDBasicOrderInfo* TDEngineInfoHelper::locate_writable(int local_id)
{
    int idx = local_id % TD_AVAILABLE_ORDER_LIMIT;
    int count = 0;
    char status = info->orders[idx].status;
    while (order_is_existing(status) && count < TD_AVAILABLE_ORDER_LIMIT)
    {
        idx = (idx + 1) % TD_AVAILABLE_ORDER_LIMIT;
        status = info->orders[idx].status;
        count ++;
    }
    if (count == TD_AVAILABLE_ORDER_LIMIT)
        return nullptr;
    else
    {
        if (idx > info->last_local_index)
            info->last_local_index = idx;
        return &(info->orders[idx]);
    }
}

void TDEngineInfoHelper::switch_day()
{
    cleanup();
}

void TDEngineInfoHelper::cleanup()
{
    info->last_local_index = -1;
    memset(info->orders, '\0', TD_AVAILABLE_ORDER_LIMIT * sizeof(TDBasicOrderInfo));
}

void TDEngineInfoHelper::set_order_status(int local_id, char status)
{
    TDBasicOrderInfo* order = locate_readable(local_id);
    if (order != nullptr)
    {
        order->status = status;
    }
}

int TDEngineInfoHelper::get_order_id(int local_id)
{
    TDBasicOrderInfo* order = locate_readable(local_id);
    if (order != nullptr)
        return order->order_id;
    else
        return -1;
}

void TDEngineInfoHelper::record_order(int local_id, int order_id)
{
    TDBasicOrderInfo* order = locate_writable(local_id);
    if (order != nullptr)
    {
        order->order_id = order_id;
        order->local_id = local_id;
        order->status = LF_CHAR_OrderInserted;
    }
}