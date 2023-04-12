// SPDX-License-Identifier: Apache-2.0

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::journal {
reader::~reader() { journals_.clear(); }

void reader::join(const data::location_ptr &location, uint32_t dest_id, const int64_t from_time) {
  auto key = static_cast<uint64_t>(location->uid) << 32u | static_cast<uint64_t>(dest_id);
  auto result = journals_.try_emplace(key, location, dest_id, false, lazy_);
  if (result.second) {
    journals_.at(key).seek_to_time(from_time);
  }
  if (current_ == nullptr) {
    sort(); // do not sort if current_ is set (because we could be in process of reading)
  }
}

void reader::disjoin(const uint32_t location_uid) {
  for (auto it = journals_.begin(); it != journals_.end();) {
    if (it->first >> 32u xor location_uid) {
      it++;
    } else {
      it = journals_.erase(it);
    }
  }
  current_ = nullptr;
  sort();
}

void reader::disjoin_channel(uint32_t location_uid, uint32_t dest_id) {
  auto key = static_cast<uint64_t>(location_uid) << 32u | static_cast<uint64_t>(dest_id);
  for (auto it = journals_.begin(); it != journals_.end();) {
    if (it->first != key) {
      it++;
    } else {
      journals_.erase(it);
      break; // only one journal erased
    }
  }
  current_ = nullptr;
  sort();
}

bool reader::data_available() {
  sort();
  return current_ != nullptr && current_frame()->has_data();
}

void reader::seek_to_time(int64_t nanotime) {
  for (auto &pair : journals_) {
    pair.second.seek_to_time(nanotime);
  }
  sort();
}

void reader::next() {
  if (current_ != nullptr) {
    current_->next();
  }
  sort();
}

void reader::sort() {
  int64_t min_time = time::now_in_nano();
  for (auto &pair : journals_) {
    auto &journal = pair.second;
    auto &frame = journal.current_frame();
    if (frame->has_data() && frame->gen_time() <= min_time) {
      min_time = frame->gen_time();
      current_ = &journal;
    }
  }
}
} // namespace kungfu::yijinjing::journal