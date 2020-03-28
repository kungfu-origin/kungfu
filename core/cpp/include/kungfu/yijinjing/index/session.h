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
typedef std::vector<longfist::types::Session> SessionVector;

class session_finder {
public:
  explicit session_finder(const yijinjing::io_device_ptr &io_device);

  virtual ~session_finder();

  virtual int64_t find_last_seen_time(const data::location_ptr &source_location);

  SessionVector find_sessions(int64_t from = 0, int64_t to = INT64_MAX);

  SessionVector find_sessions_for(const data::location_ptr &source_location, int64_t from = 0, int64_t to = INT64_MAX);

protected:
  yijinjing::io_device_ptr io_device_;
  cache::SessionStorageType session_storage_;
};

class session_builder : public session_finder {
public:
  explicit session_builder(const yijinjing::io_device_ptr &io_device);

  int64_t find_last_seen_time(const data::location_ptr &source_location) override;

  longfist::types::Session &open_session(const data::location_ptr &source_location, int64_t time);

  void close_session(const data::location_ptr &source_location, int64_t time);

  void update_session(const journal::frame_ptr &frame);

  void rebuild_index_db();

private:
  std::unordered_map<uint32_t, longfist::types::Session> live_sessions_ = {};
};
} // namespace kungfu::yijinjing::index

#endif // KUNGFU_SESSION_H
