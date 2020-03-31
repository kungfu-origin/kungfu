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
#include <kungfu/yijinjing/log.h>
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
class ipc_url_factory : public url_factory {
public:
  [[nodiscard]] std::string make_path_bind(data::location_ptr location, protocol p) const override {
    return location->locator->layout_file(location, layout::NANOMSG, get_protocol_name(p));
  }

  [[nodiscard]] std::string make_path_connect(data::location_ptr location, protocol p) const override {
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
  }

  bool is_usable() override { return publish("{}") > 0; }
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
    : home_(std::move(home)), low_latency_(low_latency), lazy_(lazy) {
  if (spdlog::default_logger()->name().empty()) {
    yijinjing::log::setup_log(home_, home_->name);
  }
  ensure_sqlite_initilize();

  live_home_ = location::make_shared(mode::LIVE, home_->category, home_->group, home_->name, home_->locator);
  url_factory_ = std::make_shared<ipc_url_factory>();
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

io_device_with_reply::io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy)
    : io_device(std::move(home), low_latency, lazy) {
  rep_sock_ = std::make_shared<nanomsg::socket>(protocol::REPLY);
  rep_sock_->bind(url_factory_->make_path_bind(home_, protocol::REPLY));
}

io_device_master::io_device_master(data::location_ptr home, bool low_latency)
    : io_device_with_reply(std::move(home), low_latency, false) {
  publisher_ = std::make_shared<nanomsg_publisher_master>(*this, is_low_latency());
  observer_ = std::make_shared<nanomsg_observer_master>(*this, is_low_latency());
}

io_device_client::io_device_client(data::location_ptr home, bool low_latency)
    : io_device_with_reply(std::move(home), low_latency, true) {}

bool io_device_client::is_usable() {
  nanomsg_publisher_client publisher(*this, false);
  nanomsg_observer_client observer(*this, false);
  return publisher.is_usable() and observer.is_usable();
}

void io_device_client::setup() {
  publisher_ = std::make_shared<nanomsg_publisher_client>(*this, is_low_latency());
  observer_ = std::make_shared<nanomsg_observer_client>(*this, is_low_latency());
  std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_RECV_TIMEOUT));
}
} // namespace kungfu::yijinjing
