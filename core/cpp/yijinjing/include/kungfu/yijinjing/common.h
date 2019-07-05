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

#include <utility>
#include <fmt/format.h>
#include <rxcpp/rx.hpp>

#include <kungfu/common.h>
#include <kungfu/yijinjing/util/util.h>

namespace kungfu
{
    namespace yijinjing
    {
        /** size related */
        constexpr int KB = 1024;
        constexpr int MB = KB * KB;

        class yijinjing_error : public std::runtime_error
        {
        public:
            explicit yijinjing_error(const std::string &message) : runtime_error(message)
            {}
        };

        class event
        {
        public:

            virtual ~event() = default;

            virtual int64_t gen_time() const = 0;

            virtual int64_t trigger_time() const = 0;

            virtual int32_t msg_type() const = 0;

            virtual uint32_t source() const = 0;

            virtual uint32_t dest() const = 0;

            virtual uint32_t data_length() const = 0;

            virtual const char *data_as_bytes() const = 0;

            virtual const std::string data_as_string() const = 0;

            virtual const std::string to_string() const = 0;

            /**
             * Using auto with the return mess up the reference with the undlerying memory address, DO NOT USE it.
             * @tparam T
             * @return a casted reference to the underlying memory address
             */
            template<typename T>
            inline const T &data() const
            {
                return *(reinterpret_cast<const T *>(data_address()));
            }

        protected:
            virtual const void *data_address() const = 0;
        };

        DECLARE_PTR(event)

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

            enum class layout : int8_t
            {
                JOURNAL,
                SQLITE,
                NANOMSG,
                LOG
            };

            inline std::string get_layout_name(layout l)
            {
                switch(l)
                {
                    case layout::JOURNAL:
                        return "journal";
                    case layout::SQLITE:
                        return "db";
                    case layout::NANOMSG:
                        return "nn";
                    case layout::LOG:
                        return "log";
                }
            }

            FORWARD_DECLARE_PTR(location)

            FORWARD_DECLARE_PTR(locator)

            class locator
            {
            public:
                locator() = default;

                virtual ~locator() = default;

                virtual const std::string layout_dir(location_ptr location, layout l) const = 0;

                virtual const std::string layout_file(location_ptr location, layout l, const std::string &name) const = 0;

                virtual const std::string default_to_system_db(location_ptr location, const std::string &name) const = 0;

                virtual const std::vector<int> list_page_id(location_ptr location, uint32_t dest_id) const = 0;
            };

            class location : public std::enable_shared_from_this<location>
            {
            public:
                location(data::mode m, data::category c, std::string g, std::string n, locator_ptr l) :
                        mode(m), category(c), group(std::move(g)), name(std::move(n)), locator(std::move(l)),
                        uname(fmt::format("{}/{}/{}/{}", data::get_category_name(category), group, name, data::get_mode_name(mode))),
                        uid(util::hash_str_32(uname))
                {}

                virtual ~location() = default;

                const mode mode;
                const category category;
                const std::string group;
                const std::string name;
                const std::string uname;
                const uint32_t uid;
                const locator_ptr locator;

                static inline location_ptr make(data::mode m, data::category c, std::string g, std::string n, locator_ptr l)
                {
                    return std::make_shared<location>(m, c, g, n, l);
                }
            };
        }
    }

    namespace rx
    {
        using namespace rxcpp;
        using namespace rxcpp::operators;
        using namespace rxcpp::util;

        auto is = [](int32_t msg_type)
        {
            return filter([=](yijinjing::event_ptr e)
                          {
                              return e->msg_type() == msg_type;
                          });
        };

        auto from = [](uint32_t source)
        {
            return filter([=](yijinjing::event_ptr e)
                          {
                              return e->source() == source;
                          });
        };

        auto trace = []()
        {
            return map([=](yijinjing::event_ptr e)
                       {
                           printf("coming event %d\n", e->msg_type());
                           return e;
                       });
        };

        template<class... ArgN>
        auto $(ArgN &&... an) -> decltype(subscribe<yijinjing::event_ptr>(std::forward<ArgN>(an)...))
        {
            return subscribe<yijinjing::event_ptr>(std::forward<ArgN>(an)...);
        }
    }
}

#endif //KUNGFU_YIJINJING_COMMON_H
