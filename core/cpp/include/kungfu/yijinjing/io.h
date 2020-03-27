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

//
// Created by Keren Dong on 2019-06-01.
//

#ifndef KUNGFU_IO_H
#define KUNGFU_IO_H

#include <sqlite3.h>

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

namespace kungfu::yijinjing {
FORWARD_DECLARE_PTR(session)

class io_device : public resource {
public:
  io_device(data::location_ptr home, bool low_latency, bool lazy);

  virtual ~io_device() = default;

  bool is_usable() override { return publisher_ and observer_ and publisher_->is_usable() and observer_->is_usable(); }

  void setup() override {
    publisher_->setup();
    observer_->setup();
  }

  [[nodiscard]] const data::locator_ptr &get_locator() const { return home_->locator; }

  [[nodiscard]] const data::location_ptr &get_home() const { return home_; }

  [[nodiscard]] const data::location_ptr &get_live_home() const { return live_home_; }

  [[nodiscard]] bool is_low_latency() { return low_latency_; }

  journal::reader_ptr open_reader_to_subscribe();

  journal::reader_ptr open_reader(const data::location_ptr &location, uint32_t dest_id);

  journal::writer_ptr open_writer(uint32_t dest_id);

  journal::writer_ptr open_writer_at(const data::location_ptr &location, uint32_t dest_id);

  nanomsg::socket_ptr connect_socket(const data::location_ptr &location, const nanomsg::protocol &p, int timeout = 0);

  nanomsg::socket_ptr bind_socket(const nanomsg::protocol &p, int timeout = 0);

  [[nodiscard]] nanomsg::url_factory_ptr get_url_factory() const { return url_factory_; }

  [[nodiscard]] publisher_ptr get_publisher() { return publisher_; }

  [[nodiscard]] observer_ptr get_observer() { return observer_; }

protected:
  data::location_ptr home_;
  data::location_ptr live_home_;
  const bool low_latency_;
  const bool lazy_;
  nanomsg::url_factory_ptr url_factory_;
  publisher_ptr publisher_;
  observer_ptr observer_;
};

DECLARE_PTR(io_device)

class io_device_with_reply : public io_device {
public:
  io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy);

  [[nodiscard]] nanomsg::socket_ptr get_rep_sock() const { return rep_sock_; }

protected:
  nanomsg::socket_ptr rep_sock_;
};

DECLARE_PTR(io_device_with_reply)

class io_device_master : public io_device_with_reply {
public:
  io_device_master(data::location_ptr home, bool low_latency);
};

DECLARE_PTR(io_device_master)

class io_device_client : public io_device_with_reply {
public:
  io_device_client(data::location_ptr home, bool low_latency);
};

DECLARE_PTR(io_device_client)

class io_device_console : public io_device {
public:
  io_device_console(data::location_ptr home, int32_t console_width, int32_t console_height);

  void trace(int64_t begin_time, int64_t end_time, bool in, bool out);

private:
  int32_t console_width_;
  int32_t console_height_;
};

DECLARE_PTR(io_device_console)

class session {
public:
  explicit session(data::location_ptr location) : location_(std::move(location)) {}

  ~session() = default;

  void update(const journal::frame_ptr &frame);

private:
  const data::location_ptr location_;
  int64_t begin_time_ = 0;
  int64_t end_time_ = -1;
  int frame_count_ = 0;
  uint64_t data_size_ = 0;
  std::vector<uint32_t> write_to_;
  std::vector<uint32_t> read_from_;

  friend io_device_master;
};

void handle_sql_error(int rc, const std::string &error_tip);
void exec_sql(sqlite3 *db, char **db_error_msg, const std::string &sql, const std::string &error_tip);
void ensure_sqlite_initilize();
void ensure_sqlite_shutdown();
} // namespace kungfu::yijinjing
#endif // KUNGFU_IO_H
