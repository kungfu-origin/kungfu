#include <utility>

#include <utility>

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

#include <kungfu/common.h>
#include <kungfu/yijinjing/util/util.h>

#define KF_DIR_SOCKET "socket"
#define KF_DIR_JOURNAL "journal"
#define KF_DIR_LOG "log"

#define DECLARE_PTR_UNI(X) typedef std::unique_ptr<X> X##_ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR_UNI(X) class X; DECLARE_PTR_UNI(X)      /** forward defile smart ptr */

#define DECLARE_PTR(X) typedef std::shared_ptr<X> X##_ptr;   /** define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /** forward defile smart ptr */

namespace kungfu
{
    namespace yijinjing
    {

        /** size related */
        const int KB = 1024;
        const int MB = KB * KB;
        const int JOURNAL_PAGE_SIZE = 128 * MB;
        const int PAGE_MIN_HEADROOM = 1 * MB;

        enum MsgType
        {
            SessionStart = 10001,
            SessionEnd = 10002,
            Subscribe = 10003,
            Unsubscribe = 10004
        };

        class event
        {
        public:

            virtual ~event() = default;

            virtual int64_t gen_time() const = 0;

            virtual int64_t trigger_time() const = 0;

            virtual int16_t msg_type() const = 0;

            virtual int16_t source() const = 0;

            template<typename T>
            inline const T &data() const
            {
                return *(reinterpret_cast<const T *>(data_address()));
            }

        protected:
            virtual const void *data_address() const = 0;
        };

        class publisher
        {
        public:

            virtual ~publisher() = default;

            virtual int notify() = 0;

            virtual int publish(const std::string &json_message) = 0;
        };

        DECLARE_PTR(publisher)

        class observer
        {
        public:

            virtual ~observer() = default;

            virtual bool wait() = 0;

            virtual const std::string &get_notice() = 0;
        };

        DECLARE_PTR(observer)

        namespace data
        {

            enum class mode : int8_t
            {
                LIVE,
                DATA,
                REPLAY,
                BACKTEST
            };

            inline std::string get_mode_name(mode m)
            {
                switch (m)
                {
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

            enum class category : int8_t
            {
                MD,
                TD,
                STRATEGY,
                SYSTEM
            };

            inline std::string get_category_name(category c)
            {
                switch (c)
                {
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

            typedef std::function<void (const uint32_t, const int)> get_page_path;


            FORWARD_DECLARE_PTR(location)
            class location
            {
            public:
                location(data::mode m, data::category c, std::string group, std::string name) :
                        mode(m), category(c), group(std::move(group)), name(std::move(name))
                {};

                virtual ~location() = default;

                const mode mode;
                const category category;
                const std::string group;
                const std::string name;

                inline uint32_t hash() const
                {
                    return util::hash_str_32(journal_path());
                }

                virtual const std::string journal_path() const = 0;
                virtual const std::string socket_path() const = 0;
                virtual const std::string log_path() const = 0;
                virtual const std::string make_path(const std::string& parent, const std::string& filename) const = 0;
                virtual const std::vector<int> list_page_id(uint32_t dest_id) const = 0;

                virtual const location_ptr make_location(data::mode m, data::category c, const std::string &group, const std::string &name) const = 0;
            };
        }

        namespace action
        {
            struct Subscribe
            {
                uint32_t source_id;
                int64_t from_time;
            };
        }
    }
}

#endif //KUNGFU_YIJINJING_COMMON_H
