//
// Created by Keren Dong on 2020/3/27.
//

#ifndef KUNGFU_SESSION_H
#define KUNGFU_SESSION_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/cache/backend.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/time.h>

namespace kungfu::yijinjing::index {
class session_keeper {
public:
  explicit session_keeper(yijinjing::io_device_ptr io_device);

  longfist::types::Session &open_session(const data::location_ptr &source_location, int64_t time);

  longfist::types::Session &close_session(const data::location_ptr &source_location, int64_t time);

  longfist::types::Session &update_session(uint32_t source, const journal::frame_ptr& frame);

  std::vector<longfist::types::Session> find_sessions(uint32_t source, int64_t from, int64_t to);

private:
  cache::SessionStorageType session_storage_;
  std::unordered_map<uint32_t, longfist::types::Session> live_sessions_ = {};
};
} // namespace kungfu::yijinjing::index

#endif // KUNGFU_SESSION_H
