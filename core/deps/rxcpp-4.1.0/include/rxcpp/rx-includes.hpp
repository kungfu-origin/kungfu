// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_INCLUDES_HPP)
#define RXCPP_RX_INCLUDES_HPP

#include "rx-trace.hpp"

// some configuration macros
#if defined(_MSC_VER)

#if _MSC_VER > 1600
#pragma warning(disable: 4348) // false positives on : redefinition of default parameter : parameter 2
#define RXCPP_USE_RVALUEREF 1
#endif

#if _MSC_VER >= 1800
#define RXCPP_USE_VARIADIC_TEMPLATES 1
#endif

#if _CPPRTTI
#define RXCPP_USE_RTTI 1
#endif

#elif defined(__clang__)

#if __has_feature(cxx_rvalue_references)
#define RXCPP_USE_RVALUEREF 1
#endif
#if __has_feature(cxx_rtti)
#define RXCPP_USE_RTTI 1
#endif
#if __has_feature(cxx_variadic_templates)
#define RXCPP_USE_VARIADIC_TEMPLATES 1
#endif

#elif defined(__GNUG__)

#define GCC_VERSION (__GNUC__ * 10000 + \
                     __GNUC_MINOR__ * 100 + \
                     __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40801
#define RXCPP_USE_RVALUEREF 1
#endif

#if GCC_VERSION >= 40400
#define RXCPP_USE_VARIADIC_TEMPLATES 1
#endif

#if defined(__GXX_RTTI)
#define RXCPP_USE_RTTI 1
#endif

#endif

//
// control std::hash<> of enum
// force with RXCPP_FORCE_HASH_ENUM & RXCPP_FORCE_HASH_ENUM_UNDERLYING
// in time use ifdef to detect library support for std::hash<> of enum
//
#define RXCPP_HASH_ENUM 0
#define RXCPP_HASH_ENUM_UNDERLYING 1

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define RXCPP_USE_WINRT 0
#else
#define RXCPP_USE_WINRT 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
#define RXCPP_ON_IOS
#endif
#endif

#if defined(__ANDROID__)
#define RXCPP_ON_ANDROID
#endif

#if defined(RXCPP_FORCE_USE_VARIADIC_TEMPLATES)
#undef RXCPP_USE_VARIADIC_TEMPLATES
#define RXCPP_USE_VARIADIC_TEMPLATES RXCPP_FORCE_USE_VARIADIC_TEMPLATES
#endif

#if defined(RXCPP_FORCE_USE_RVALUEREF)
#undef RXCPP_USE_RVALUEREF
#define RXCPP_USE_RVALUEREF RXCPP_FORCE_USE_RVALUEREF
#endif

#if defined(RXCPP_FORCE_USE_RTTI)
#undef RXCPP_USE_RTTI
#define RXCPP_USE_RTTI RXCPP_FORCE_USE_RTTI
#endif

#if defined(RXCPP_FORCE_USE_WINRT)
#undef RXCPP_USE_WINRT
#define RXCPP_USE_WINRT RXCPP_FORCE_USE_WINRT
#endif

#if defined(RXCPP_FORCE_HASH_ENUM)
#undef RXCPP_HASH_ENUM
#define RXCPP_HASH_ENUM RXCPP_FORCE_HASH_ENUM
#endif

#if defined(RXCPP_FORCE_HASH_ENUM_UNDERLYING)
#undef RXCPP_HASH_ENUM_UNDERLYING
#define RXCPP_HASH_ENUM_UNDERLYING RXCPP_FORCE_HASH_ENUM_UNDERLYING
#endif

#if defined(RXCPP_FORCE_ON_IOS)
#undef RXCPP_ON_IOS
#define RXCPP_ON_IOS RXCPP_FORCE_ON_IOS
#endif

#if defined(RXCPP_FORCE_ON_ANDROID)
#undef RXCPP_ON_ANDROID
#define RXCPP_ON_ANDROID RXCPP_FORCE_ON_ANDROID
#endif

#if defined(_MSC_VER) && !RXCPP_USE_VARIADIC_TEMPLATES
// resolve args needs enough to store all the possible resolved args
#define _VARIADIC_MAX 10
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1800)
#define RXCPP_NOEXCEPT
#else
#define RXCPP_NOEXCEPT noexcept
#endif

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <stdlib.h>

#include <cstddef>

#include <iostream>
#include <iomanip>

#include <exception>
#include <functional>
#include <memory>
#include <array>
#include <vector>
#include <algorithm>
#include <atomic>
#include <map>
#include <set>
#include <mutex>
#include <deque>
#include <thread>
#include <future>
#include <list>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <initializer_list>
#include <typeinfo>
#include <tuple>
#include <unordered_set>
#include <type_traits>
#include <utility>

#if defined(RXCPP_ON_IOS) || defined(RXCPP_ON_ANDROID)
#include <pthread.h>
#endif

#include "rx-util.hpp"
#include "rx-predef.hpp"
#include "rx-subscription.hpp"
#include "rx-observer.hpp"
#include "rx-scheduler.hpp"
#include "rx-subscriber.hpp"
#include "rx-notification.hpp"
#include "rx-coordination.hpp"
#include "rx-sources.hpp"
#include "rx-subjects.hpp"
#include "rx-operators.hpp"
#include "rx-observable.hpp"
#include "rx-connectable_observable.hpp"
#include "rx-grouped_observable.hpp"

#if !defined(RXCPP_LITE)
#include "operators/rx-all.hpp"
#include "operators/rx-amb.hpp"
#include "operators/rx-any.hpp"
#include "operators/rx-buffer_count.hpp"
#include "operators/rx-buffer_time.hpp"
#include "operators/rx-buffer_time_count.hpp"
#include "operators/rx-combine_latest.hpp"
#include "operators/rx-concat.hpp"
#include "operators/rx-concat_map.hpp"
#include "operators/rx-connect_forever.hpp"
#include "operators/rx-debounce.hpp"
#include "operators/rx-delay.hpp"
#include "operators/rx-distinct.hpp"
#include "operators/rx-distinct_until_changed.hpp"
#include "operators/rx-element_at.hpp"
#include "operators/rx-filter.hpp"
#include "operators/rx-finally.hpp"
#include "operators/rx-flat_map.hpp"
#include "operators/rx-group_by.hpp"
#include "operators/rx-ignore_elements.hpp"
#include "operators/rx-map.hpp"
#include "operators/rx-merge.hpp"
#include "operators/rx-merge_delay_error.hpp"
#include "operators/rx-observe_on.hpp"
#include "operators/rx-on_error_resume_next.hpp"
#include "operators/rx-pairwise.hpp"
#include "operators/rx-reduce.hpp"
#include "operators/rx-repeat.hpp"
#include "operators/rx-replay.hpp"
#include "operators/rx-retry.hpp"
#include "operators/rx-sample_time.hpp"
#include "operators/rx-scan.hpp"
#include "operators/rx-sequence_equal.hpp"
#include "operators/rx-skip.hpp"
#include "operators/rx-skip_while.hpp"
#include "operators/rx-skip_last.hpp"
#include "operators/rx-skip_until.hpp"
#include "operators/rx-start_with.hpp"
#include "operators/rx-subscribe_on.hpp"
#include "operators/rx-switch_if_empty.hpp"
#include "operators/rx-switch_on_next.hpp"
#include "operators/rx-take.hpp"
#include "operators/rx-take_last.hpp"
#include "operators/rx-take_until.hpp"
#include "operators/rx-take_while.hpp"
#include "operators/rx-tap.hpp"
#include "operators/rx-time_interval.hpp"
#include "operators/rx-timeout.hpp"
#include "operators/rx-timestamp.hpp"
#include "operators/rx-window.hpp"
#include "operators/rx-window_time.hpp"
#include "operators/rx-window_time_count.hpp"
#include "operators/rx-window_toggle.hpp"
#include "operators/rx-with_latest_from.hpp"
#include "operators/rx-zip.hpp"
#endif

#pragma pop_macro("min")
#pragma pop_macro("max")

#endif
