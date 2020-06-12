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

#ifndef YIJINJING_JOURNAL_H
#define YIJINJING_JOURNAL_H

#include <mutex>

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::journal {
/**
 * Journal class, the abstraction of continuous memory access
 */
class journal {
public:
  journal(data::location_ptr location, uint32_t dest_id, bool is_writing, bool lazy)
      : location_(std::move(location)), dest_id_(dest_id), is_writing_(is_writing), lazy_(lazy),
        frame_(std::shared_ptr<frame>(new frame())), page_frame_nb_(0u) {}

  ~journal();

  [[nodiscard]] frame_ptr &current_frame() { return frame_; }

  [[nodiscard]] page_ptr &current_page() { return page_; }

  /**
   * move current frame to the next available one
   */
  void next();

  /**
   * makes sure after this call, next time user calls current_frame() gets the right available frame
   * (gen_time() > nanotime or writable)
   * @param nanotime
   */
  void seek_to_time(int64_t nanotime);

private:
  const data::location_ptr location_;
  const uint32_t dest_id_;
  const bool is_writing_;
  const bool lazy_;
  page_ptr page_;
  frame_ptr frame_;
  uint64_t page_frame_nb_;

  void load_page(int page_id);

  /** load next page, current page will be released if not empty */
  void load_next_page();

  friend class reader;

  friend class writer;
};

class reader {
public:
  explicit reader(bool lazy) : lazy_(lazy), current_(nullptr){};

  ~reader();

  /**
   * join journal at given data location
   * @param location where the journal locates
   * @param dest_id journal dest id
   * @param from_time subscribe events after this time, 0 means from start
   */
  void join(const data::location_ptr &location, uint32_t dest_id, int64_t from_time);

  void disjoin(uint32_t location_uid);

  [[nodiscard]] frame_ptr current_frame() const { return current_->current_frame(); }

  [[nodiscard]] page_ptr current_page() const { return current_->current_page(); }

  bool data_available();

  /** seek journal to time */
  void seek_to_time(int64_t nanotime);

  /** seek next frame */
  void next();

  void sort();

private:
  const bool lazy_;
  journal *current_;
  std::unordered_map<uint64_t, journal> journals_;
};

class writer {
public:
  writer(const data::location_ptr &location, uint32_t dest_id, bool lazy, publisher_ptr publisher);

  [[nodiscard]] const data::location_ptr &get_location() const { return journal_.location_; }

  [[nodiscard]] uint32_t get_dest() const { return journal_.dest_id_; }

  uint64_t current_frame_uid();

  frame_ptr open_frame(int64_t trigger_time, int32_t msg_type, uint32_t length);

  void close_frame(size_t data_length, int64_t gen_time = time::now_in_nano());

  void copy_frame(const frame_ptr &source);

  void mark(int64_t trigger_time, int32_t msg_type);

  void mark_at(int64_t gen_time, int64_t trigger_time, int32_t msg_type);

  void write_raw(int64_t trigger_time, int32_t msg_type, uintptr_t data, uint32_t length);

  /**
   * Using auto with the return mess up the reference with the undlerying memory address, DO NOT USE it.
   * @tparam T
   * @param trigger_time
   * @param msg_type
   * @return a casted reference to the underlying memory address in mmap file
   */
  template <typename T> std::enable_if_t<size_fixed_v<T>, T &> open_data(int64_t trigger_time = 0) {
    auto frame = open_frame(trigger_time, T::tag, sizeof(T));
    return const_cast<T &>(frame->template data<T>());
  }

  void close_data();

  template <typename T>
  std::enable_if_t<size_fixed_v<T>> write(int64_t trigger_time, const T &data, int32_t msg_type = T::tag) {
    auto frame = open_frame(trigger_time, msg_type, sizeof(T));
    auto size = frame->copy_data(data);
    close_frame(size);
  }

  template <typename T>
  std::enable_if_t<size_unfixed_v<T>> write(int64_t trigger_time, const T &data, int32_t msg_type = T::tag) {
    auto s = data.to_string();
    auto size = s.length();
    auto frame = open_frame(trigger_time, msg_type, size);
    memcpy(const_cast<void *>(frame->data_address()), s.c_str(), size);
    close_frame(size);
  }

  template <typename T>
  std::enable_if_t<size_fixed_v<T>> write_as(int64_t trigger_time, const T &data, uint32_t source, uint32_t dest) {
    auto frame = open_frame(trigger_time, T::tag, sizeof(T));
    auto size = frame->copy_data(data);
    frame->set_source(source);
    frame->set_dest(dest);
    close_frame(size);
  }

  template <typename T>
  std::enable_if_t<size_unfixed_v<T>> write_as(int64_t trigger_time, const T &data, uint32_t source, uint32_t dest) {
    auto s = data.to_string();
    auto size = s.length();
    auto frame = open_frame(trigger_time, T::tag, size);
    memcpy(const_cast<void *>(frame->data_address()), s.c_str(), size);
    frame->set_source(source);
    frame->set_dest(dest);
    close_frame(size);
  }

  template <typename T>
  std::enable_if_t<size_fixed_v<T>> write_at(int64_t gen_time, int64_t trigger_time, const T &data) {
    auto frame = open_frame(trigger_time, T::tag, sizeof(T));
    auto size = frame->copy_data(data);
    close_frame(size, gen_time);
  }

  template <typename T>
  std::enable_if_t<size_unfixed_v<T>> write_at(int64_t gen_time, int64_t trigger_time, const T &data) {
    auto s = data.to_string();
    auto size = s.length();
    auto frame = open_frame(trigger_time, T::tag, size);
    memcpy(const_cast<void *>(frame->data_address()), s.c_str(), size);
    close_frame(size, gen_time);
  }

private:
  const uint64_t frame_id_base_;
  journal journal_;
  std::mutex writer_mtx_ = {};
  publisher_ptr publisher_;
  size_t size_to_write_;

  void close_page(int64_t trigger_time);
};
} // namespace kungfu::yijinjing::journal
#endif // YIJINJING_JOURNAL_H
