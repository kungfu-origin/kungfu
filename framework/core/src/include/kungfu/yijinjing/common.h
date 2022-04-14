/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
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

#include <csignal>
#include <cstdarg>
#include <filesystem>
#include <rxcpp/rx.hpp>

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/util/stacktrace.h>
#include <kungfu/yijinjing/util/util.h>

namespace kungfu {
namespace yijinjing {
/** size related */
constexpr int KB = 1024;
constexpr int MB = KB * KB;

class yijinjing_error : public std::runtime_error {
public:
  explicit yijinjing_error(const std::string &message) : runtime_error(message) {}
};

class resource {
public:
  virtual bool is_usable() = 0;

  virtual void setup() = 0;
};

class publisher : public resource {
public:
  virtual ~publisher() = default;

  virtual int notify() = 0;

  virtual int publish(const std::string &json_message) = 0;
};

DECLARE_PTR(publisher)

class observer : public resource {
public:
  virtual ~observer() = default;

  virtual bool wait() = 0;

  virtual const std::string &get_notice() = 0;
};

DECLARE_PTR(observer)

namespace data {
FORWARD_DECLARE_STRUCT_PTR(location)
FORWARD_DECLARE_CLASS_PTR(locator)
typedef std::unordered_map<uint32_t, location_ptr> location_map;

class locator {
public:
  locator();

  explicit locator(const std::string &root) : root_(root) {}

  virtual ~locator() = default;

  [[nodiscard]] virtual bool has_env(const std::string &name) const;

  [[nodiscard]] virtual std::string get_env(const std::string &name) const;

  [[nodiscard]] virtual std::string layout_dir(const location_ptr &location, longfist::enums::layout layout) const;

  [[nodiscard]] virtual std::string layout_file(const location_ptr &location, longfist::enums::layout layout,
                                                const std::string &name) const;

  [[nodiscard]] virtual std::string default_to_system_db(const location_ptr &location, const std::string &name) const;

  [[nodiscard]] virtual std::vector<uint32_t> list_page_id(const location_ptr &location, uint32_t dest_id) const;

  [[nodiscard]] virtual std::vector<location_ptr> list_locations(const std::string &category, const std::string &group,
                                                                 const std::string &name,
                                                                 const std::string &mode) const;

  [[nodiscard]] virtual std::vector<uint32_t> list_location_dest(const location_ptr &location) const;

private:
  std::filesystem::path root_;
};

struct location : public std::enable_shared_from_this<location>, public longfist::types::Location {
  static constexpr uint32_t PUBLIC = 0;
  static constexpr uint32_t SYNC = 1;

  const locator_ptr locator;
  const std::string uname;
  const uint32_t uid;

  location(longfist::enums::mode m, longfist::enums::category c, std::string g, std::string n, locator_ptr l)
      : locator(std::move(l)), uname(fmt::format("{}/{}/{}/{}", longfist::enums::get_category_name(c), g, n,
                                                 longfist::enums::get_mode_name(m))),
        uid(util::hash_str_32(uname)) {
    category = c;
    group = std::move(g);
    name = std::move(n);
    mode = m;
    location_uid = uid;
  }

  template <typename T> inline T to() const {
    T t{};
    t.location_uid = uid;
    t.mode = mode;
    t.category = category;
    t.group = group;
    t.name = name;
    return t;
  }

  template <typename T> inline T &to(T &t) const {
    t.location_uid = uid;
    t.mode = mode;
    t.category = category;
    t.group = group;
    t.name = name;
    return t;
  }

  template <typename T> static inline location_ptr make_shared(T msg, locator_ptr l) {
    return std::make_shared<location>(msg.mode, msg.category, msg.group, msg.name, l);
  }

  static inline location_ptr make_shared(longfist::enums::mode m, longfist::enums::category c, std::string g,
                                         std::string n, locator_ptr l) {
    return std::make_shared<location>(m, c, g, n, l);
  }
};
} // namespace data
} // namespace yijinjing

namespace hana {
using namespace boost::hana;

template <typename T> nlohmann::json to_json(T &obj) {
  nlohmann::json j{};
  hana::for_each(hana::accessors<T>(), [&](auto t) { j[hana::first(t).c_str()] = hana::second(t)(obj); });
  return j;
}
} // namespace hana

namespace rx {
using namespace rxcpp;
using namespace rxcpp::operators;
using namespace rxcpp::util;

static constexpr auto noop_event_handler = []() { return [](const event_ptr &event) {}; };

static constexpr auto error_handler_log = [](const std::string &subscriber_name) {
  return [=](const std::exception_ptr &e) {
    try {
      std::rethrow_exception(e);
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("subscriber {} got error: {}", subscriber_name, ex.what());
    }
  };
};

[[maybe_unused]] static constexpr auto complete_handler_log = [](const std::string &subscriber_name) {
  return [=]() { SPDLOG_DEBUG("subscriber {} completed", subscriber_name); };
};

template <typename EventType>
static constexpr auto instanceof
    = []() { return filter([](const event_ptr &event) { return dynamic_cast<EventType *>(event.get()) != nullptr; }); };

template <typename... Ts>
static constexpr auto event_filter_any = [](auto member) {
  return [=](Ts... arg) {
    using T = std::invoke_result_t<decltype(member), event *>;
    type_check<T, Ts...>(arg...);
    auto args = boost::hana::make_tuple(arg...);
    return filter([=](const event_ptr &event) {
      auto check = [&](auto a) { return ((*event).*member)() == a; };
      return boost::hana::fold(boost::hana::transform(args, check), std::logical_or<>());
    });
  };
};

template <typename... Ts> constexpr decltype(auto) is(Ts... arg) {
  return event_filter_any<Ts...>(&event::msg_type)(arg...);
}

template <typename... Ts> constexpr decltype(auto) from(Ts... arg) {
  return event_filter_any<Ts...>(&event::source)(arg...);
}

template <typename... Ts> constexpr decltype(auto) to(Ts... arg) {
  return event_filter_any<Ts...>(&event::dest)(arg...);
}

static constexpr auto interrupt_on_error = [](const std::exception_ptr &e) {
  try {
    std::rethrow_exception(e);
  } catch (const rx::empty_error &ex) {
    SPDLOG_WARN("{}", ex.what());
  } catch (const std::exception &ex) {
    SPDLOG_ERROR("Unexpected exception {} by rx:subscriber {}", typeid(ex).name(), ex.what());
    yijinjing::util::print_stack_trace();
  }
  raise(SIGINT);
};

template <class Arg>
static constexpr auto $(Arg an) -> decltype(subscribe<event_ptr>(std::forward<Arg>(an), interrupt_on_error)) {
  return subscribe<event_ptr>(std::forward<Arg>(an), interrupt_on_error);
}

template <class... ArgN>
static constexpr auto $(ArgN &&...an) -> decltype(subscribe<event_ptr>(std::forward<ArgN>(an)...)) {
  return subscribe<event_ptr>(std::forward<ArgN>(an)...);
}

template <class T, class Observable, class Subject> struct steppable : public operator_base<T> {
  typedef decay_t<Observable> source_type;
  typedef decay_t<Subject> subject_type;

  struct steppable_state : public std::enable_shared_from_this<steppable_state> {
    steppable_state(source_type o, subject_type sub) : source(std::move(o)), subject_value(std::move(sub)) {}

    source_type source;
    subject_type subject_value;
    rxu::detail::maybe<typename composite_subscription::weak_subscription> connection;
  };

  std::shared_ptr<steppable_state> state;

  steppable(source_type o, subject_type sub) : state(std::make_shared<steppable_state>(std::move(o), std::move(sub))) {}

  template <class Subscriber> void on_subscribe(Subscriber &&o) const {
    state->subject_value.get_observable().subscribe(std::forward<Subscriber>(o));
  }

  void on_connect(composite_subscription cs) const {
    auto destination = state->subject_value.get_subscriber();
    if (state->connection.empty()) {

      // the lifetime of each connect is nested in the subject lifetime
      state->connection.reset(destination.add(cs));

      auto localState = state;

      // when the connection is finished it should shutdown the connection
      cs.add([destination, localState]() {
        if (!localState->connection.empty()) {
          destination.remove(localState->connection.get());
          localState->connection.reset();
        }
      });
    }
    // use cs not destination for lifetime of subscribe.
    state->source.subscribe(cs, destination);
  }
};

template <class Observable = rx::observable<event_ptr>, class SourceValue = rx::util::value_type_t<Observable>,
          class Subject = rx::subjects::subject<SourceValue>,
          class Steppable = rx::steppable<SourceValue, rx::util::decay_t<Observable>, Subject>,
          class Result = rx::connectable_observable<SourceValue, Steppable>>
std::function<Result(Observable)> holdon() {
  return [&](Observable ob) { return Result(Steppable(ob, Subject(rx::composite_subscription()))); };
}
} // namespace rx
} // namespace kungfu

#define $$(handler) $([&](const event_ptr &event) { handler; })

#endif // KUNGFU_YIJINJING_COMMON_H
