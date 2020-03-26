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
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#define DEFAULT_RECV_TIMEOUT 25
#define DEFAULT_NOTICE_TIMEOUT 1000

using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu::yijinjing {
class local_sqlite_session_keeper : public sqlite_session_keeper {
public:
  local_sqlite_session_keeper() = default;

  void open(sqlite3 *index_db) override {
    int rc;

    rc = sqlite3session_create(index_db, "main", &db_session_);
    handle_sql_error(rc, "failed to create index db session");

    rc = sqlite3session_attach(db_session_, nullptr);
    handle_sql_error(rc, "failed to attach index db session");
  }

  void close() override {
    int rc = sqlite3session_changeset(db_session_, &db_changeset_nb_, &db_changeset_ptr_);
    handle_sql_error(rc, "failed to collect index db session changeset");
    SPDLOG_TRACE("index db changeset nb={}", db_changeset_nb_);
    //            sqlite3changeset_apply(
    //                    index_db_,
    //                    db_changeset_nb_, db_changeset_ptr_,
    //                    0, xConflict,
    //                    (void*)1
    //            );
    sqlite3session_delete(db_session_);
  }

private:
  sqlite3_session *db_session_ = nullptr;
  int db_changeset_nb_ = 0;
  void *db_changeset_ptr_ = nullptr;
};

class ipc_url_factory : public url_factory {
public:
  [[nodiscard]] const std::string make_path_bind(data::location_ptr location, protocol p) const override {
    return location->locator->layout_file(location, layout::NANOMSG, get_protocol_name(p));
  }

  [[nodiscard]] const std::string make_path_connect(data::location_ptr location, protocol p) const override {
    return location->locator->layout_file(location, layout::NANOMSG, get_protocol_name(get_opposite_protol(p)));
  }
};

class nanomsg_resource : public resource {
protected:
  nanomsg_resource(const io_device &io_device, bool low_latency, protocol p)
      : io_device_(io_device), low_latency_(low_latency),
        location_(std::make_shared<data::location>(longfist::enums::mode::LIVE, longfist::enums::category::SYSTEM,
                                                   "master", "master", io_device_.get_home()->locator)),
        bind_path_(io_device_.get_url_factory()->make_path_bind(location_, p)),
        connect_path_(io_device_.get_url_factory()->make_path_connect(location_, p)), socket_(p) {}

  const io_device &io_device_;
  const bool low_latency_;
  const location_ptr location_;
  const std::string bind_path_;
  const std::string connect_path_;
  nanomsg::socket socket_;
};

class nanomsg_publisher : public publisher, protected nanomsg_resource {
public:
  nanomsg_publisher(const io_device &io_device, bool low_latency, protocol p)
      : nanomsg_resource(io_device, low_latency, p) {}

  ~nanomsg_publisher() override { socket_.close(); }

  void setup() override {}

  int notify() override { return low_latency_ ? 0 : publish("{}"); }

  int publish(const std::string &json_message) override { return socket_.send(json_message); }
};

class nanomsg_publisher_master : public nanomsg_publisher {
public:
  nanomsg_publisher_master(const io_device &io_device, bool low_latency)
      : nanomsg_publisher(io_device, low_latency, protocol::PUBLISH) {
    socket_.bind(bind_path_);
  }

  bool is_usable() override { return true; }
};

class nanomsg_publisher_client : public nanomsg_publisher {
public:
  nanomsg_publisher_client(const io_device &io_device, bool low_latency)
      : nanomsg_publisher(io_device, low_latency, protocol::PUSH) {
    socket_.connect(connect_path_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  bool is_usable() override {
    auto now = time::now_in_nano();
    nlohmann::json ping;
    ping["msg_type"] = Ping::tag;
    ping["gen_time"] = now;
    ping["trigger_time"] = now;
    ping["source"] = 0;
    ping["dest"] = 0;
    return socket_.send(ping.dump()) > 0;
  }
};

class nanomsg_observer : public observer, protected nanomsg_resource {
public:
  nanomsg_observer(const io_device &io_device, bool low_latency, protocol p)
      : nanomsg_resource(io_device, low_latency, p), recv_flags_(low_latency ? NN_DONTWAIT : 0) {
    socket_.setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, DEFAULT_RECV_TIMEOUT);
  }

  ~nanomsg_observer() override { socket_.close(); }

  void setup() override {
    if (not low_latency_) {
      socket_.setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, DEFAULT_NOTICE_TIMEOUT);
    }
  }

  bool wait() override { return socket_.recv(recv_flags_) > 0; }

  const std::string &get_notice() override { return socket_.last_message(); }

private:
  int recv_flags_;
};

class nanomsg_observer_master : public nanomsg_observer {
public:
  nanomsg_observer_master(const io_device &io_device, bool low_latency)
      : nanomsg_observer(io_device, low_latency, protocol::PULL) {
    socket_.bind(bind_path_);
  }

  bool is_usable() override { return true; }
};

class nanomsg_observer_client : public nanomsg_observer {
public:
  nanomsg_observer_client(const io_device &io_device, bool low_latency)
      : nanomsg_observer(io_device, low_latency, protocol::SUBSCRIBE) {
    socket_.connect(connect_path_);
    socket_.setsockopt_str(NN_SUB, NN_SUB_SUBSCRIBE, "");
  }

  bool is_usable() override { return socket_.recv(0) > 0; }
};

io_device::io_device(data::location_ptr home, const bool low_latency, const bool lazy)
    : home_(std::move(home)), low_latency_(low_latency), lazy_(lazy),
      ssk_(std::make_shared<local_sqlite_session_keeper>()) {
  if (spdlog::default_logger()->name().empty()) {
    yijinjing::log::setup_log(home_, home_->name);
  }
  ensure_sqlite_initilize();

  db_home_ = location::make_shared(mode::LIVE, category::SYSTEM, "journal", "index", home_->locator);
  live_home_ = location::make_shared(mode::LIVE, home_->category, home_->group, home_->name, home_->locator);
  url_factory_ = std::make_shared<ipc_url_factory>();

  auto index_db_file = home_->locator->layout_file(db_home_, layout::SQLITE, "index");

  int rc = sqlite3_open_v2(index_db_file.c_str(), &index_db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  handle_sql_error(rc, "failed to open index db");

  exec_sql(index_db_, &db_error_msg_, "PRAGMA journal_mode=WAL;", "failed to set index db to WAL");
  exec_sql(index_db_, &db_error_msg_, "create table if not exists session (id INTEGER PRIMARY KEY, data JSON);",
           "failed to create tables");

  ssk_->open(index_db_);

  sqlite3_prepare_v2(index_db_,
                     R"(
select data from session
where json_extract(session.data, '$.uid') between ?1 and ?2
and json_extract(session.data, '$.begin_time') >= ?3
and json_extract(session.data, '$.end_time') <= ?4;
)",
                     -1, &stmt_find_sessions_, nullptr);
}

io_device::~io_device() {
  sqlite3_finalize(stmt_find_sessions_);
  ssk_->close();
  sqlite3_close_v2(index_db_);
  SPDLOG_TRACE("index db closed");
}

reader_ptr io_device::open_reader_to_subscribe() { return std::make_shared<reader>(lazy_); }

reader_ptr io_device::open_reader(const data::location_ptr &location, uint32_t dest_id) {
  auto r = std::make_shared<reader>(lazy_);
  r->join(location, dest_id, 0);
  return r;
}

writer_ptr io_device::open_writer(uint32_t dest_id) {
  return std::make_shared<writer>(home_, dest_id, lazy_, publisher_);
}

writer_ptr io_device::open_writer_at(const data::location_ptr &location, uint32_t dest_id) {
  return std::make_shared<writer>(location, dest_id, lazy_, publisher_);
}

socket_ptr io_device::connect_socket(const data::location_ptr &location, const protocol &p, int timeout) {
  socket_ptr s = std::make_shared<nanomsg::socket>(p);
  s->connect(url_factory_->make_path_connect(location, p));
  s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
  SPDLOG_INFO("connected socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), location->name,
              s->get_url(), timeout);
  return s;
}

socket_ptr io_device::bind_socket(const protocol &p, int timeout) {
  socket_ptr s = std::make_shared<nanomsg::socket>(p);
  s->bind(url_factory_->make_path_bind(home_, p));
  s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
  SPDLOG_INFO("bind to socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), home_->name, s->get_url(),
              timeout);
  return s;
}

std::vector<std::string> io_device::find_sessions(uint32_t source, int64_t from, int64_t to) const {
  std::vector<std::string> result;
  sqlite3_reset(stmt_find_sessions_);
  if (source > 0) {
    sqlite3_bind_int64(stmt_find_sessions_, 1, source);
    sqlite3_bind_int64(stmt_find_sessions_, 2, source);
  } else {
    sqlite3_bind_int64(stmt_find_sessions_, 1, 0);
    sqlite3_bind_int64(stmt_find_sessions_, 2, INT64_MAX);
  }
  sqlite3_bind_int64(stmt_find_sessions_, 3, from);
  sqlite3_bind_int64(stmt_find_sessions_, 4, to);
  SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_find_sessions_));
  int rc = sqlite3_step(stmt_find_sessions_);
  while (SQLITE_ROW == rc) {
    result.emplace_back((const char *)sqlite3_column_text(stmt_find_sessions_, 0));
    rc = sqlite3_step(stmt_find_sessions_);
  }
  if (SQLITE_ERROR == rc) {
    SPDLOG_ERROR("error occurred when query sessions: {}", sqlite3_errmsg(index_db_));
  }
  return result;
}

io_device_with_reply::io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy)
    : io_device(std::move(home), low_latency, lazy) {
  rep_sock_ = std::make_shared<nanomsg::socket>(protocol::REPLY);
  rep_sock_->bind(url_factory_->make_path_bind(home_, protocol::REPLY));
}

io_device_master::io_device_master(data::location_ptr home, bool low_latency)
    : io_device_with_reply(std::move(home), low_latency, false) {
  publisher_ = std::make_shared<nanomsg_publisher_master>(*this, low_latency);
  observer_ = std::make_shared<nanomsg_observer_master>(*this, low_latency);

  sqlite3_prepare_v2(index_db_, R"(delete from session;)", -1, &stmt_clean_sessions_, nullptr);
  sqlite3_prepare_v2(
      index_db_,
      R"(insert into session (data) values (json_set('{}', '$.uid', ?1, '$.location', ?2, '$.begin_time', ?3, '$.end_time', 0));)",
      -1, &stmt_open_session_, nullptr);
  sqlite3_prepare_v2(index_db_,
                     R"(
update session set data = json_set(session.data, '$.end_time', ?3, '$.frame_count', ?4, '$.data_size', ?5)
where json_extract(session.data, '$.uid') = ?1 and json_extract(session.data, '$.begin_time') = ?2;
)",
                     -1, &stmt_close_session_, nullptr);
}

void io_device_master::open_session(const location_ptr &location, int64_t time) {
  auto uid = location->uid;
  sessions_[uid] = std::make_shared<session>(location);
  sessions_[uid]->begin_time_ = time;

  sqlite3_reset(stmt_open_session_);
  sqlite3_bind_int64(stmt_open_session_, 1, uid);
  sqlite3_bind_text(stmt_open_session_, 2, location->uname.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int64(stmt_open_session_, 3, time);
  SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_open_session_));
  sqlite3_step(stmt_open_session_);
  SPDLOG_INFO("{} opened session [{}, )", location->uname, time::strftime(time));
}

void io_device_master::update_session(const frame_ptr &frame) {
  if (sessions_.find(frame->source()) != sessions_.end()) {
    auto &&session = sessions_[frame->source()];
    session->update(frame);
  } else {
    SPDLOG_ERROR("no session found for {:08x}", frame->source());
  }
}

void io_device_master::close_session(const data::location_ptr &location, int64_t time) {
  auto uid = location->uid;
  auto session = sessions_[uid];
  sqlite3_reset(stmt_close_session_);
  sqlite3_bind_int64(stmt_close_session_, 1, uid);
  sqlite3_bind_int64(stmt_close_session_, 2, session->begin_time_);
  sqlite3_bind_int64(stmt_close_session_, 3, time);
  sqlite3_bind_int(stmt_close_session_, 4, session->frame_count_);
  sqlite3_bind_int(stmt_close_session_, 5, session->data_size_);
  SPDLOG_TRACE("[sql] {}", sqlite3_expanded_sql(stmt_close_session_));
  sqlite3_step(stmt_close_session_);
  SPDLOG_INFO("{} closed session [{}, {}]", session->location_->uname, time::strftime(session->begin_time_),
              time::strftime(time));
}

void io_device_master::rebuild_index_db() {
  sqlite3_reset(stmt_clean_sessions_);
  sqlite3_step(stmt_clean_sessions_);

  std::unordered_map<uint32_t, location_ptr> locations;
  std::unordered_set<uint32_t> master_cmd_uids;
  auto reader = open_reader_to_subscribe();
  for (const auto &location : home_->locator->list_locations()) {
    SPDLOG_TRACE("investigating location {}", location->uname);
    locations[location->uid] = location;
    if (location->category == longfist::enums::category::SYSTEM && location->group == "master" &&
        location->name != "master") {
      master_cmd_uids.insert(location->uid);
    }
    for (const auto dest_uid : home_->locator->list_location_dest(location)) {
      reader->join(location, dest_uid, 0);
    }
  }
  while (reader->data_available()) {
    auto &&frame = reader->current_frame();
    auto &&location = locations[frame->dest() == 0 ? frame->source() : frame->dest()];
    switch (frame->msg_type()) {
    case SessionStart::tag: {
      open_session(location, frame->gen_time());
      break;
    }
    case SessionEnd::tag: {
      close_session(location, frame->gen_time());
      break;
    }
    default: {
      if (master_cmd_uids.find(frame->source()) == master_cmd_uids.end()) {
        update_session(frame);
      }
      break;
    }
    }
    reader->next();
  }
}

io_device_client::io_device_client(data::location_ptr home, bool low_latency)
    : io_device_with_reply(std::move(home), low_latency, true) {
  publisher_ = std::make_shared<nanomsg_publisher_client>(*this, low_latency);
  observer_ = std::make_shared<nanomsg_observer_client>(*this, low_latency);
}

void session::update(const frame_ptr &frame) {
  end_time_ = frame->gen_time();
  frame_count_++;
  data_size_ += frame->frame_length();
}
} // namespace kungfu::yijinjing
