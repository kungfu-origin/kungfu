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

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::journal {

journal::~journal() {
  if (page_.get() != nullptr) {
    page_.reset();
  }
}

void journal::next() {
  assert(page_.get() != nullptr);
  if (frame_->msg_type() == longfist::types::PageEnd::tag) {
    load_next_page();
  } else {
    frame_->move_to_next();
    page_frame_nb_++;
  }
}

void journal::seek_to_time(int64_t nanotime) {
  int page_id = page::find_page_id(location_, dest_id_, nanotime);
  load_page(page_id);
  while (page_->is_full() && page_->end_time() <= nanotime) {
    load_next_page();
  }
  while (frame_->has_data() && frame_->gen_time() <= nanotime) {
    next();
  }
}

void journal::load_page(int page_id) {
  if (page_.get() == nullptr or page_->get_page_id() != page_id) {
    page_ = page::load(location_, dest_id_, page_id, is_writing_, lazy_);
    frame_->set_address(page_->first_frame_address());
    page_frame_nb_ = 0u;
  }
}

void journal::load_next_page() { load_page(page_->get_page_id() + 1); }
} // namespace kungfu::yijinjing::journal