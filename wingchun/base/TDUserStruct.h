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

#ifndef WINGCHUN_TDENGINESTRUCT_H
#define WINGCHUN_TDENGINESTRUCT_H

#include "WC_DECLARE.h"
#include "constants.h"
#include "json.hpp"

using json = nlohmann::json;

WC_NAMESPACE_START

#define ORDER_INFO_RAW  '\0'
#define ORDER_INFO_TICKER_LIMIT 15

struct TDOrderInfo
{
    /** request id of the order */
    int  order_id;
    /** assigned by TradeEngine */
    int  local_id;
    /** LfOrderStatusType, ORDER_INFO_RAW means nothing */
    char status;
    /** InstrumentID name */
    char ticker[ORDER_INFO_TICKER_LIMIT];
} __attribute__((packed));

/** for one single strategy,
 * how many available orders can be supported.
 * available order */
#define AVAILABLE_ORDER_LIMIT  10000
/** enough space to store json for pos map */
#define POS_JSON_STR_LENGTH  100000

/**
 * Trade engine user info
 * will contain info for risk management.
 */
#define TD_INFO_NORMAL        'n'
#define TD_INFO_WRITING       'w'
#define TD_INFO_FORCE_QUIT    'e'

struct TDUserInfo
{
    /** TD_INFO_NORMAL / TD_INFO_WRITING / TD_INFO_FORCE_QUIT, others raw */
    char status;
    /** last start time */
    long start_time;
    /** last end time */
    long end_time;
    /** last order request id (located order index)*/
    int  last_order_index;
    /** strategy name */
    char name[JOURNAL_SHORT_NAME_MAX_LENGTH];
    /** folder name */
    char folder[JOURNAL_FOLDER_MAX_LENGTH];
    /** strategy-wise position */
    char pos_str[POS_JSON_STR_LENGTH];
    /** order info hash-map base */
    TDOrderInfo orders[AVAILABLE_ORDER_LIMIT];

} __attribute__((packed));

#define TD_USER_INFO_FOLDER KUNGFU_JOURNAL_FOLDER "user/" /**< where we put user info files */
/** user info file length is determined */
const int USER_INFO_SIZE = sizeof(TDUserInfo) + 1024;

class TDUserInfoHelper
{
    /** all method that may modify data must be private
     * to make it modifiable by ITDEngine and td_pos_tool only */
    friend class ITDEngine;
    friend bool set_pos_tool(const string& user_name, short source, const string& file_name, bool is_csv);

private:
    /** internal map: user_name -> memory */
    map<string, TDUserInfo*> address_book;
    /** source */
    short source;

protected:
    /** internal locate at readable order with order_id, return nullptr if no available */
    TDOrderInfo* locate_readable(const string& user_name, int order_id) const;
    /** internal locate at writable order position with order_id, return nullptr if no available */
    TDOrderInfo* locate_writable(const string& user_name, int order_id);

protected:
    /** constructor with "write" authority */
    TDUserInfoHelper(short source);
    /** load user info of user_name */
    void load(const string& user_name, bool need_write=true);
    /** remove user from list, detach memory */
    void remove(const string& user_name);
    /** necessary cleanup when to a trading day is over */
    void switch_day();
    /** record order info */
    void record_order(const string& user_name, int local_id, int order_id, const char* ticker);
    /** set position json string */
    void set_pos(const string& user_name, const json& pos);
    /** clean up user info */
    void clean_up(TDUserInfo* info);
    /** get info block address for internal usage only */
    inline TDUserInfo* get_user_info(const string& user_name) const
    {
        auto iter = address_book.find(user_name);
        if (iter == address_book.end())
            return nullptr;
        return iter->second;
    }

public:
    /** constructor with read-ability only */
    TDUserInfoHelper(short source, const string& user_name);
    /** get const user info */
    const TDUserInfo* get_const_user_info(const string& user_name) const;
    /** default destructor */
    ~TDUserInfoHelper();
    /** get order info, return true if local_id and ticker are set properly */
    bool get_order(const string& user_name, int order_id, int &local_id, char* ticker) const;
    /** set order status when updated by return-order */
    void set_order_status(const string& user_name, int order_id, char status);
    /** get position json */
    json get_pos(const string& user_name) const;
    /** get existing orders of this user */
    vector<int> get_existing_orders(const string& user_name) const;
};

DECLARE_PTR(TDUserInfoHelper);

/** for whole td engine,
 * how many orders can be supported.
 * available order */
#define TD_AVAILABLE_ORDER_LIMIT  100000

struct TDBasicOrderInfo
{
    /** request id of the order */
    int  order_id;
    /** assigned by TradeEngine */
    int  local_id;
    /** LfOrderStatusType, ORDER_INFO_RAW means nothing */
    char status;
} __attribute__((packed));

/** VERSION */
#define TD_ENGINE_INFO_VERSION 1

struct TDEngineInfo
{
    /** TD_INFO_NORMAL / TD_INFO_WRITING / TD_INFO_FORCE_QUIT, others raw */
    char status;
    /** version */
    short version;
    /** start time */
    long start_time;
    /** last end time */
    long end_time;
    /** last order local id */
    int  last_local_index;
    /** source id name */
    short source;
    /** orders by local_id */
    TDBasicOrderInfo orders[TD_AVAILABLE_ORDER_LIMIT];
} __attribute__((packed));

const int TD_ENGINE_INFO_SIZE = sizeof(TDEngineInfo) + 1024;

class TDEngineInfoHelper
{
    /** all method that may modify data must be private
     * to make it modifiable by ITDEngine only */
    friend class ITDEngine;

private:
    TDEngineInfo* info;

protected:
    TDBasicOrderInfo* locate_readable(int local_id) const;
    TDBasicOrderInfo* locate_writable(int local_id);
    void cleanup();

protected:
    TDEngineInfoHelper(short source, const string& name);

public:
    /** default destructor */
    ~TDEngineInfoHelper();
    /** necessary cleanup when to a trading day is over */
    void switch_day();
    /** set order status when updated by return-order */
    void set_order_status(int local_id, char status);
    /** get order_id */
    int get_order_id(int local_id);
    /** insert order info */
    void record_order(int local_id, int order_id);
};

DECLARE_PTR(TDEngineInfoHelper);

WC_NAMESPACE_END

#endif //WINGCHUN_TDENGINESTRUCT_H
