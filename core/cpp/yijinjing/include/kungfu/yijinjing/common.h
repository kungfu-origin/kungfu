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

#ifndef KUNGFU_YIJINJING_COMMON_H
#define KUNGFU_YIJINJING_COMMON_H

#include <typeinfo>
#include <nlohmann/json.hpp>

#include <kungfu/common.h>

#define KF_DIR_SOCKET "socket"
#define KF_DIR_JOURNAL "journal"
#define KF_DIR_LOG "log"

#define __FRAME_HEADER_VERSION__ 3

//////////////////////////////////////////
/// (byte) JournalFrameStatus
//////////////////////////////////////////
#define JOURNAL_FRAME_STATUS_RAW        0
#define JOURNAL_FRAME_STATUS_WRITTEN    1
#define JOURNAL_FRAME_STATUS_PAGE_END   2

//////////////////////////////////////////
/// (byte) JournalFrameLastFlag
//////////////////////////////////////////
#define JOURNAL_FRAME_NOT_LAST          0 /**< false */
#define JOURNAL_FRAME_IS_LAST           1 /**< true */

/** reserve space for page header (long) */
#define PAGE_HEADER_RESERVE 10
//////////////////////////////////////////
/// (byte) JournalPageStatus
//////////////////////////////////////////
#define JOURNAL_PAGE_STATUS_RAW     0
#define JOURNAL_PAGE_STATUS_INITED  1

#define PAGE_PATH_MAX_LENGTH       1024

#define PAGE_SERVICE_MSG_FILE "PAGE_SERVICE_MSG"

/** max number of communication users in the same time */
#define MAX_MEMORY_MSG_NUMBER 1000
/** REQUEST_ID_RANGE * MAX_MEMORY_MSG_NUMBER < 2147483647(max num of int) */
#define REQUEST_ID_RANGE 1000000

#define DECLARE_PTR_UNI(X) typedef std::unique_ptr<X> X##_ptr;   /** define smart ptr */
#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##_ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /** forward defile smart ptr */

#define JOURNAL_PREFIX std::string("yjj")        /** journal file prefix */
#define JOURNAL_SUFFIX std::string("journal")    /** journal file suffix */

/** fast type convert for moving address forward */
#define ADDRESS_ADD(x, delta) (void*)((uintptr_t)x + delta)

namespace kungfu {
    namespace yijinjing {

        /** seed of hash function */
        const uint32_t HASH_SEED = 97;

        /** size related */
        const int KB = 1024;
        const int MB = KB * KB;
        const int JOURNAL_PAGE_SIZE = 128 * MB;
        const int PAGE_MIN_HEADROOM = 1 * MB;

        enum MsgType {
            SessionStart = 10001,
            SessionEnd = 10002
        };

        namespace data {

            enum class mode : int8_t {
                LIVE,
                DATA,
                REPLAY,
                BACKTEST
            };

            inline std::string get_mode_name(mode m) {
                switch (m) {
                    case mode::LIVE:
                        return "live";
                    case mode::DATA:
                        return "data";
                    case mode::REPLAY:
                        return "replay";
                    case mode::BACKTEST:
                        return "backtest";
                }
            }

            enum class category : int8_t {
                MD,
                TD,
                STRATEGY,
                SYSTEM
            };

            inline std::string get_category_name(category c) {
                switch (c) {
                    case category::MD:
                        return "md";
                    case category::TD:
                        return "td";
                    case category::STRATEGY:
                        return "strategy";
                    case category::SYSTEM:
                        return "system";
                }
            }
        }

        class event {
        public:
            virtual int64_t gen_time() const = 0;
            virtual int64_t trigger_time() const = 0;
            virtual int16_t msg_type() const = 0;
            virtual int16_t source() const = 0;

            template <typename T>
            inline const T& data() const {
                return *(reinterpret_cast<const T *>(data_address()));
            }

        protected:
            virtual const void * data_address() const = 0;
        };

        /**
         * Murmur Hash 2
         * @param key content to be hashed
         * @param len length of key
         * @param seed
         * @return hash result
         */
        inline uint32_t MurmurHash2(const void *key, int32_t len, uint32_t seed) {
            // 'm' and 'r' are mixing constants generated offline.
            // They're not really 'magic', they just happen to work well.

            const uint32_t m = 0x5bd1e995;
            const int32_t r = 24;

            // Initialize the hash to a 'random' value

            uint32_t h = seed ^len;

            // Mix 4 bytes at a time into the hash

            const unsigned char *data = (const unsigned char *) key;

            while (len >= 4) {
                uint32_t k = *(uint32_t *) data;

                k *= m;
                k ^= k >> r;
                k *= m;

                h *= m;
                h ^= k;

                data += 4;
                len -= 4;
            }

            // Handle the last few bytes of the input array

            switch (len) {
                case 3:
                    h ^= data[2] << 16;
                case 2:
                    h ^= data[1] << 8;
                case 1:
                    h ^= data[0];
                    h *= m;
            };

            // Do a few final mixes of the hash to ensure the last few
            // bytes are well-incorporated.

            h ^= h >> 13;
            h *= m;
            h ^= h >> 15;

            return h;
        }
    }
}

#endif //KUNGFU_YIJINJING_COMMON_H
