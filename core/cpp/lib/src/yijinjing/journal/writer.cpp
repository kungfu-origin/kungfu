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
#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::journal {
constexpr uint32_t PAGE_ID_TRANC = 0xFFFF0000;
constexpr uint32_t FRAME_ID_TRANC = 0x0000FFFF;

writer::writer(const data::location_ptr &location, uint32_t dest_id, bool lazy, publisher_ptr publisher)
    : frame_id_base_(uint64_t(location->uid xor dest_id) << 32u), journal_(location, dest_id, true, lazy),
      publisher_(std::move(publisher)), size_to_write_(0) {
  journal_.seek_to_time(time::now_in_nano());
}

uint64_t writer::current_frame_uid() {
  uint32_t page_part = (journal_.page_->page_id_ << 16u) & PAGE_ID_TRANC;
  uint32_t frame_part = journal_.page_frame_nb_ & FRAME_ID_TRANC;
  return frame_id_base_ | (page_part | frame_part);
}

frame_ptr writer::open_frame(int64_t trigger_time, int32_t msg_type, uint32_t data_length) {
  assert(sizeof(frame_header) + data_length + sizeof(frame_header) <= journal_.page_->get_page_size());
  int64_t start_time = time::now_in_nano();
  while (not writer_mtx_.try_lock()) {
    if (time::now_in_nano() - start_time > time_unit::NANOSECONDS_PER_MILLISECOND) {
      throw journal_error("Can not lock writer for " + journal_.location_->uname);
    }
  }
  if (journal_.current_frame()->address() + sizeof(frame_header) + data_length > journal_.page_->address_border()) {
    close_page(trigger_time);
  }
  auto frame = journal_.current_frame();
  frame->set_header_length();
  frame->set_trigger_time(trigger_time);
  frame->set_msg_type(msg_type);
  frame->set_source(journal_.location_->uid);
  frame->set_dest(journal_.dest_id_);
  size_to_write_ = data_length;
  return frame;
}

void writer::close_frame(size_t data_length) {
  assert(size_to_write_ >= data_length);
  auto frame = journal_.current_frame();
  auto next_frame_address = frame->address() + frame->header_length() + data_length;
  assert(next_frame_address < journal_.page_->address_border());
  memset(reinterpret_cast<void *>(next_frame_address), 0, sizeof(frame_header));
  frame->set_gen_time(time::now_in_nano());
  frame->set_data_length(data_length);
  size_to_write_ = 0;
  journal_.page_->set_last_frame_position(frame->address() - journal_.page_->address());
  journal_.next();
  writer_mtx_.unlock();
  publisher_->notify();
}

void writer::mark(int64_t trigger_time, int32_t msg_type) {
  open_frame(trigger_time, msg_type, 0);
  close_frame(0);
}

void writer::mark_with_time(int64_t gen_time, int32_t msg_type) {
  assert(sizeof(frame_header) + sizeof(frame_header) <= journal_.page_->get_page_size());
  if (journal_.current_frame()->address() + sizeof(frame_header) > journal_.page_->address_border()) {
    mark(gen_time, longfist::types::PageEnd::tag);
    journal_.load_next_page();
  }
  auto frame = journal_.current_frame();
  frame->set_header_length();
  frame->set_trigger_time(gen_time);
  frame->set_msg_type(msg_type);
  frame->set_source(journal_.location_->uid);
  frame->set_dest(journal_.dest_id_);
  memset(reinterpret_cast<void *>(frame->address() + frame->header_length()), 0, sizeof(frame_header));
  frame->set_gen_time(gen_time);
  frame->set_data_length(0);
  journal_.page_->set_last_frame_position(frame->address() - journal_.page_->address());
  journal_.next();
}

void writer::write_raw(int64_t trigger_time, int32_t msg_type, uintptr_t data, uint32_t length) {
  auto frame = open_frame(trigger_time, msg_type, length);
  memcpy(const_cast<void *>(frame->data_address()), reinterpret_cast<void *>(data), length);
  close_frame(length);
}

template <> void writer::write(int64_t trigger_time, int32_t msg_type, const std::string &data) {
  write_raw(trigger_time, msg_type, reinterpret_cast<uintptr_t>(data.c_str()), data.length());
}

void writer::close_page(int64_t trigger_time) {
  page_ptr last_page = journal_.page_;
  journal_.load_next_page();

  frame last_page_frame;
  last_page_frame.set_address(last_page->last_frame_address());
  last_page_frame.move_to_next();
  last_page_frame.set_header_length();
  last_page_frame.set_trigger_time(trigger_time);
  last_page_frame.set_msg_type(longfist::types::PageEnd::tag);
  last_page_frame.set_source(journal_.location_->uid);
  last_page_frame.set_dest(journal_.dest_id_);
  last_page_frame.set_gen_time(time::now_in_nano());
  last_page_frame.set_data_length(0);
  last_page->set_last_frame_position(last_page_frame.address() - last_page->address());
}
} // namespace kungfu::yijinjing::journal
