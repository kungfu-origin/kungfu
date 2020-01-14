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
#include <typeinfo>
#include <signal.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
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

            [[nodiscard]] virtual int64_t gen_time() const = 0;

            [[nodiscard]] virtual int64_t trigger_time() const = 0;

            [[nodiscard]] virtual int32_t msg_type() const = 0;

            [[nodiscard]] virtual uint32_t source() const = 0;

            [[nodiscard]] virtual uint32_t dest() const = 0;

            [[nodiscard]] virtual uint32_t data_length() const = 0;

            [[nodiscard]] virtual const char *data_as_bytes() const = 0;

            [[nodiscard]] virtual std::string data_as_string() const = 0;

            [[nodiscard]] virtual std::string to_string() const = 0;

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
                    default:
                        return "live";
                }
            }

            inline mode get_mode_by_name(const std::string &name)
            {
                if (name == "live")
                    return mode::LIVE;
                else if (name == "data")
                    return mode::DATA;
                else if (name == "replay")
                    return mode::REPLAY;
                else if (name == "backtest")
                    return mode::BACKTEST;
                else
                    return mode::LIVE;
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
                    default:
                        return "system";
                }
            }

            inline category get_category_by_name(const std::string &name)
            {
                if (name == "md")
                    return category::MD;
                else if (name == "td")
                    return category::TD;
                else if (name == "strategy")
                    return category::STRATEGY;
                else
                    return category::SYSTEM;
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
                switch (l)
                {
                    case layout::JOURNAL:
                        return "journal";
                    case layout::SQLITE:
                        return "db";
                    case layout::NANOMSG:
                        return "nn";
                    case layout::LOG:
                        return "log";
                    default:
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

                [[nodiscard]] virtual bool has_env(const std::string &name) const = 0;

                [[nodiscard]] virtual std::string get_env(const std::string &name) const = 0;

                [[nodiscard]] virtual std::string layout_dir(location_ptr location, layout l) const = 0;

                [[nodiscard]] virtual std::string layout_file(location_ptr location, layout l, const std::string &name) const = 0;

                [[nodiscard]] virtual std::string default_to_system_db(location_ptr location, const std::string &name) const = 0;

                [[nodiscard]] virtual std::vector<int> list_page_id(location_ptr location, uint32_t dest_id) const = 0;

                [[nodiscard]] virtual std::vector<location_ptr>
                list_locations(const std::string &category = "*", const std::string &group = "*", const std::string &name = "*",
                               const std::string &mode = "*") const = 0;

                [[nodiscard]] virtual std::vector<uint32_t> list_location_dest(location_ptr location) const = 0;
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

                const data::mode mode;
                const data::category category;
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

        inline auto is = [](int32_t msg_type)
        {
            return filter([=](yijinjing::event_ptr e)
                          {
                              return e->msg_type() == msg_type;
                          });
        };

        inline auto from = [](uint32_t source)
        {
            return filter([=](yijinjing::event_ptr e)
                          {
                              return e->source() == source;
                          });
        };

        inline auto to = [](uint32_t dest)
        {
            return filter([=](yijinjing::event_ptr e)
                          {
                              return e->dest() == dest;
                          });
        };

        inline auto trace = []()
        {
            return map([=](yijinjing::event_ptr e)
                       {
                           printf("coming event %d\n", e->msg_type());
                           return e;
                       });
        };

        inline auto interrupt_on_error(std::exception_ptr e)
        {
            try
            { std::rethrow_exception(e); }
            catch (const std::exception &ex)
            {
                SPDLOG_ERROR("Unexpected exception {} by rx:subscriber {}", typeid(ex).name(), ex.what());
            }
            raise(SIGINT);
        }

        template<class Arg>
        inline auto $(Arg an) -> decltype(subscribe<yijinjing::event_ptr>(std::forward<Arg>(an), interrupt_on_error))
        {
            return subscribe<yijinjing::event_ptr>(std::forward<Arg>(an), interrupt_on_error);
        }

        template<class... ArgN>
        inline auto $(ArgN &&... an) -> decltype(subscribe<yijinjing::event_ptr>(std::forward<ArgN>(an)...))
        {
            return subscribe<yijinjing::event_ptr>(std::forward<ArgN>(an)...);
        }

        template<class T, class Observable, class Subject>
        struct steppable : public operator_base<T>
        {
            typedef decay_t<Observable> source_type;
            typedef decay_t<Subject> subject_type;

            struct steppable_state : public std::enable_shared_from_this<steppable_state>
            {
                steppable_state(source_type o, subject_type sub)
                        : source(std::move(o)), subject_value(std::move(sub))
                {
                }

                source_type source;
                subject_type subject_value;
                rxu::detail::maybe<typename composite_subscription::weak_subscription> connection;
            };

            std::shared_ptr<steppable_state> state;

            steppable(source_type o, subject_type sub) : state(std::make_shared<steppable_state>(std::move(o), std::move(sub)))
            {}

            template<class Subscriber>
            void on_subscribe(Subscriber &&o) const
            {
                state->subject_value.get_observable().subscribe(std::forward<Subscriber>(o));
            }

            void on_connect(composite_subscription cs) const
            {
                auto destination = state->subject_value.get_subscriber();
                if (state->connection.empty())
                {

                    // the lifetime of each connect is nested in the subject lifetime
                    state->connection.reset(destination.add(cs));

                    auto localState = state;

                    // when the connection is finished it should shutdown the connection
                    cs.add(
                            [destination, localState]()
                            {
                                if (!localState->connection.empty())
                                {
                                    destination.remove(localState->connection.get());
                                    localState->connection.reset();
                                }
                            });
                }
                // use cs not destination for lifetime of subscribe.
                state->source.subscribe(cs, destination);
            }
        };

        template<
                class Observable = rx::observable <yijinjing::event_ptr>,
                class SourceValue = rx::util::value_type_t <Observable>,
                class Subject = rx::subjects::subject <SourceValue>,
                class Steppable = rx::steppable <SourceValue, rx::util::decay_t<Observable>, Subject>,
                class Result = rx::connectable_observable <SourceValue, Steppable>
        >
        std::function<Result(Observable)> holdon()
        {
            return [&](Observable ob)
            {
                return Result(Steppable(ob, Subject(rx::composite_subscription())));
            };
        }
    }
}

#endif //KUNGFU_YIJINJING_COMMON_H
