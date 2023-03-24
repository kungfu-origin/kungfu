// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_NANOMSG_SOCKET_H
#define KUNGFU_NANOMSG_SOCKET_H

#include <algorithm>
#include <cstring>
#include <exception>

#include <nng/compat/nanomsg/nn.h>
#include <nng/compat/nanomsg/pipeline.h>
#include <nng/compat/nanomsg/pubsub.h>
#include <nng/compat/nanomsg/reqrep.h>

#include <kungfu/yijinjing/common.h>

#define MAX_MSG_LENGTH (16 * 1024)

namespace kungfu::yijinjing::nanomsg {
enum class protocol : int {
  UNKNOWN = -1,
  REPLY = NN_REP,
  REQUEST = NN_REQ,
  PUSH = NN_PUSH,
  PULL = NN_PULL,
  PUBLISH = NN_PUB,
  SUBSCRIBE = NN_SUB
};

inline std::string get_protocol_name(protocol p) {
  switch (p) {
  case protocol::REPLY:
    return "rep";
  case protocol::REQUEST:
    return "req";
  case protocol::PUSH:
    return "push";
  case protocol::PULL:
    return "pull";
  case protocol::PUBLISH:
    return "pub";
  case protocol::SUBSCRIBE:
    return "sub";
  default:
    return "unknown";
  }
}

[[maybe_unused]] inline protocol get_opposite_protol(protocol p) {
  switch (p) {
  case protocol::REPLY:
    return protocol::REQUEST;
  case protocol::REQUEST:
    return protocol::REPLY;
  case protocol::PUSH:
    return protocol::PULL;
  case protocol::PULL:
    return protocol::PUSH;
  case protocol::PUBLISH:
    return protocol::SUBSCRIBE;
  case protocol::SUBSCRIBE:
    return protocol::PUBLISH;
  default:
    return protocol::UNKNOWN;
  }
}

class url_factory {
public:
  [[nodiscard]] virtual std::string make_path_bind(data::location_ptr location, protocol p) const = 0;

  [[nodiscard]] virtual std::string make_path_connect(data::location_ptr location, protocol p) const = 0;
};

DECLARE_PTR(url_factory)

class nn_exception : public std::exception {
public:
  nn_exception() : errno_(nn_errno()) {}

  [[nodiscard]] const char *what() const noexcept override;

  [[maybe_unused]] [[nodiscard]] int num() const;

private:
  int errno_;
};

DECLARE_PTR(nn_exception)

class socket {
public:
  explicit socket(protocol p) : socket(AF_SP, p, MAX_MSG_LENGTH){};

  socket(int domain, protocol p) : socket(domain, p, MAX_MSG_LENGTH){};

  socket(int domain, protocol p, int buffer_size);

  ~socket();

  void setsockopt(int level, int option, const void *optval, size_t optvallen) const;

  void setsockopt_str(int level, int option, const std::string &value) const;

  void setsockopt_int(int level, int option, int value) const;

  void getsockopt(int level, int option, void *optval, size_t *optvallen) const;

  [[maybe_unused]] [[nodiscard]] int getsockopt_int(int level, int option) const;

  int bind(const std::string &path);

  int connect(const std::string &path);

  [[maybe_unused]] void shutdown(int how = 0) const;

  void close() const;

  int send(const std::string &msg, int flags = NN_DONTWAIT) const;

  int recv(int flags = NN_DONTWAIT);

  const std::string &recv_msg(int flags = NN_DONTWAIT);

  [[maybe_unused]] [[nodiscard]] int send_json(const nlohmann::json &msg, int flags = NN_DONTWAIT) const;

  [[maybe_unused]] nlohmann::json recv_json(int flags = 0);

  const std::string &request(const std::string &json_message);

  [[maybe_unused]] [[nodiscard]] protocol get_protocol() const { return protocol_; };

  [[nodiscard]] const std::string &get_url() const { return url_; };

  [[nodiscard]] const std::string &last_message() const { return message_; };

private:
  int sock_;
  protocol protocol_;
  std::string url_;
  std::vector<char> buf_;
  std::string message_;

  /*  Prevent making copies of the socket by accident. */
  socket(const socket &);

  void operator=(const socket &);
};

DECLARE_PTR(socket)

struct nanomsg_json : event {
  explicit nanomsg_json(const std::string &msg) : binding_(nlohmann::json::parse(msg)), msg_(msg){};

  [[nodiscard]] int64_t gen_time() const override { return get_meta<int64_t>("gen_time", 0); }

  [[nodiscard]] int64_t trigger_time() const override { return get_meta<int64_t>("trigger_time", 0); }

  [[nodiscard]] int32_t msg_type() const override { return get_meta<int32_t>("msg_type", 0); }

  [[nodiscard]] uint32_t source() const override { return get_meta<uint32_t>("source", 0); }

  [[nodiscard]] uint32_t dest() const override { return get_meta<uint32_t>("dest", 0); }

  [[nodiscard]] uint32_t data_length() const override { return binding_.size(); }

  [[nodiscard]] const void *data_address() const override {
    if (binding_.find("data") == binding_.end()) {
      SPDLOG_ERROR("nanomsg event has no data {}", msg_);
      return &binding_;
    }
    return &binding_["data"];
  }

  [[nodiscard]] const char *data_as_bytes() const override { return msg_.c_str(); }

  [[nodiscard]] std::string data_as_string() const override { return binding_["data"].dump(); }

  [[nodiscard]] std::string to_string() const override { return msg_; }

private:
  const nlohmann::json binding_;
  const std::string msg_;

  template <typename T> [[nodiscard]] T get_meta(const std::string &name, T default_value) const {
    if (binding_.find(name) == binding_.end()) {
      return default_value;
    }
    T value = binding_[name];
    return value;
  }
};

DECLARE_PTR(nanomsg_json)
} // namespace kungfu::yijinjing::nanomsg

#endif // KUNGFU_NANOMSG_SOCKET_H
