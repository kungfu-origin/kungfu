/*****************************************************************************
 * Copyright [taurus.ai]
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
 * Define basic macros.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * This file is supposed to be included by all files in this project
 */

#ifndef YIJINJING_DECLARE_H
#define YIJINJING_DECLARE_H

#include <vector>
#include <map>
#include <unordered_map>

#include <kungfu/common.h>

using std::string; /** default using string */
using std::vector; /** default using vector */
using std::map;    /** default using map */

template<class Key, class T>
using umap = std::unordered_map<Key, T>; /** unordered_map usage, will merge more */

typedef unsigned char byte;
typedef unsigned int uint;
typedef std::pair<int, int> IntPair;

#define YJJ_NAMESPACE_START namespace kungfu { namespace yijinjing { /** start of yjj namespace */
#define YJJ_NAMESPACE_END }};                               /** end of yjj namespace */
#define USING_YJJ_NAMESPACE using namespace kungfu::yijinjing;

YJJ_NAMESPACE_START

/** seed of hash function */
const uint HASH_SEED = 97;

#define JOURNAL_SHORT_NAME_MAX_LENGTH   30
#define JOURNAL_FOLDER_MAX_LENGTH       100

/** seed of hash function */
const int64_t TIME_FROM_FIRST = 0;
const int64_t TIME_TO_LAST = -1;

/** size related */
const int KB = 1024;
const int MB = KB * KB;
const int JOURNAL_PAGE_SIZE = 128 * MB;
const int PAGE_MIN_HEADROOM = 2 * MB;

#define KUNGFU_FOLDER kungfu::get_kungfu_home() /** base folder of kungfu system */
#define KUNGFU_JOURNAL_FOLDER KUNGFU_FOLDER + "/journal/"  /** where we put journal files */
#define KUNGFU_SOCKET_FOLDER KUNGFU_FOLDER + "/socket/"    /** where we put socket files */
#define KUNGFU_LOG_FOLDER KUNGFU_FOLDER + "/log/"          /** where we put log files */

#define PAGED_JOURNAL_FOLDER KUNGFU_JOURNAL_FOLDER + "system/" /** paged system journal folder */
#define PAGED_JOURNAL_NAME "SYSTEM"                     /** paged system journal name */

#define KFS_SERVICE_URL "ipc://" + KUNGFU_SOCKET_FOLDER + "service.sock"
#define KFS_EMITTER_URL "ipc://" + KUNGFU_SOCKET_FOLDER + "emitter.sock"
#define KFS_NOTICE_URL "ipc://" + KUNGFU_SOCKET_FOLDER + "notice.sock"

#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##Ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /** forward defile smart ptr */

#define JOURNAL_PREFIX string("yjj")        /** journal file prefix */
#define JOURNAL_SUFFIX string("journal")    /** journal file suffix */

/** fast type convert for moving address forward */
#define ADDRESS_ADD(x, delta) (void*)((uintptr_t)x + delta)

/* system message types  0 ~ 99 */
// 0 - 9 for test
const short MSG_TYPE_PYTHON_OBJ = 0;
// 10 - 19 strategy
const short MSG_TYPE_STRATEGY_START = 10;
const short MSG_TYPE_STRATEGY_END = 11;
const short MSG_TYPE_TRADE_ENGINE_LOGIN = 12; // we don't need logout, just use strategy_end.
const short MSG_TYPE_TRADE_ENGINE_ACK = 13; // when trade engine got login request, send ack back.
const short MSG_TYPE_STRATEGY_POS_SET = 14; // strategy setup pos json.
// 20 - 29 service
const short MSG_TYPE_PAGED_START = 20;
const short MSG_TYPE_PAGED_END = 21;
// 30 - 39 control
const short MSG_TYPE_TRADE_ENGINE_OPEN = 30;
const short MSG_TYPE_TRADE_ENGINE_CLOSE = 31;
const short MSG_TYPE_MD_ENGINE_OPEN = 32;
const short MSG_TYPE_MD_ENGINE_CLOSE = 33;
const short MSG_TYPE_SWITCH_TRADING_DAY = 34;
const short MSG_TYPE_STRING_COMMAND = 35;
// 50 - 89 utilities
const short MSG_TYPE_TIME_TICK = 50;
const short MSG_TYPE_SUBSCRIBE_MARKET_DATA = 51;
const short MSG_TYPE_SUBSCRIBE_L2_MD = 52;
const short MSG_TYPE_SUBSCRIBE_INDEX = 53;
const short MSG_TYPE_SUBSCRIBE_ORDER_TRADE = 54;
const short MSG_TYPE_ENGINE_STATUS = 60;
// 90 - 99 memory alert
const short MSG_TYPE_MEMORY_FROZEN = 90; // UNLESS SOME MEMORY UNLOCK, NO MORE LOCKING

/**
 * Murmur Hash 2
 * @param key content to be hashed
 * @param len length of key
 * @param seed
 * @return hash result
 */
inline uint32_t MurmurHash2 ( const void * key, int32_t len, uint32_t seed )
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const uint32_t m = 0x5bd1e995;
    const int32_t r = 24;

    // Initialize the hash to a 'random' value

    uint32_t h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4)
    {
        uint32_t k = *(uint32_t *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

YJJ_NAMESPACE_END

#endif //YIJINJING_DECLARE_H
