//
// Created by Keren Dong on 2020/2/9.
//

#include <utility>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/practice/watcher.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::yijinjing::practice
{
    watcher::watcher(location_ptr &location, bool low_latency) : apprentice(location, low_latency)
    {
    }
}
