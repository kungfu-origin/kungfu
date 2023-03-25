// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/wingchun/strategy/context.h>

using namespace kungfu::yijinjing::practice;
using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::util;

namespace kungfu::wingchun::strategy {

bool Context::is_book_held() const { return book_held_; }

bool Context::is_positions_mirrored() const { return positions_mirrored_; }

void Context::hold_book() { book_held_ = true; }

void Context::hold_positions() { positions_mirrored_ = false; }

} // namespace kungfu::wingchun::strategy
