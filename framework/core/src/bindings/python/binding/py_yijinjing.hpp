//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_YIJINJING_HPP
#define KUNGFU_PY_YIJINJING_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/index/session.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/journal/assemble.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/practice/master.h>
#include <kungfu/yijinjing/practice/profile.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/util.h>

namespace kungfu::yijinjing {
namespace py = pybind11;

using namespace kungfu;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::longfist::enums;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::index;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;
using namespace kungfu::yijinjing::util;
using namespace kungfu::yijinjing::practice;

class PyLocator : public locator {
  using locator::locator;

  [[nodiscard]] bool has_env(const std::string &name) const override {
    PYBIND11_OVERLOAD(bool, locator, has_env, name);
  }

  [[nodiscard]] std::string get_env(const std::string &name) const override {
    PYBIND11_OVERLOAD(std::string, locator, get_env, name);
  }

  [[nodiscard]] std::string layout_dir(const location_ptr &location, layout l) const override {
    PYBIND11_OVERLOAD(std::string, locator, layout_dir, location, l);
  }

  [[nodiscard]] std::string layout_file(const location_ptr &location, layout l,
                                        const std::string &name) const override {
    PYBIND11_OVERLOAD(std::string, locator, layout_file, location, l, name);
  }

  [[nodiscard]] std::string default_to_system_db(const location_ptr &location, const std::string &name) const override {
    PYBIND11_OVERLOAD(std::string, locator, default_to_system_db, location, name);
  }

  [[nodiscard]] std::vector<uint32_t> list_page_id(const location_ptr &location, uint32_t dest_id) const override {
    PYBIND11_OVERLOAD(std::vector<uint32_t>, locator, list_page_id, location, dest_id);
  }

  [[nodiscard]] std::vector<location_ptr> list_locations(const std::string &category, const std::string &group,
                                                         const std::string &name,
                                                         const std::string &mode) const override {
    PYBIND11_OVERLOAD(std::vector<location_ptr>, locator, list_locations, category, group, name, mode);
  }

  [[nodiscard]] std::vector<uint32_t> list_location_dest(const location_ptr &location) const override {
    PYBIND11_OVERLOAD(std::vector<uint32_t>, locator, list_location_dest, location);
  }
};

class PyEvent : public event {
public:
  [[nodiscard]] int64_t gen_time() const override { PYBIND11_OVERLOAD_PURE(int64_t, event, gen_time, ) }

  [[nodiscard]] int64_t trigger_time() const override { PYBIND11_OVERLOAD_PURE(int64_t, event, trigger_time, ) }

  [[nodiscard]] int32_t msg_type() const override { PYBIND11_OVERLOAD_PURE(int64_t, event, msg_type, ) }

  [[nodiscard]] uint32_t source() const override { PYBIND11_OVERLOAD_PURE(int64_t, event, source, ) }

  [[nodiscard]] uint32_t dest() const override { PYBIND11_OVERLOAD_PURE(int64_t, event, dest, ) }
};

class PyPublisher : public publisher {
public:
  int notify() override { PYBIND11_OVERLOAD_PURE(int, publisher, notify, ) }

  int publish(const std::string &json_message) override {
    PYBIND11_OVERLOAD_PURE(int, publisher, publish, json_message)
  }
};

class PyObserver : public observer {
public:
  bool wait() override { PYBIND11_OVERLOAD_PURE(bool, observer, wait, ) }

  const std::string &get_notice() override { PYBIND11_OVERLOAD_PURE(const std::string &, observer, get_notice, ) }
};

class PySink : public sink {
public:
  void put(const data::location_ptr &location, uint32_t dest_id, const frame_ptr &frame) override {
    PYBIND11_OVERLOAD_PURE(void, sink, put, location, dest_id, frame)
  }
  void close() override { PYBIND11_OVERLOAD(void, sink, close, ); }
};

class PyMaster : public master {
public:
  using master::master;

  void on_exit() override { PYBIND11_OVERLOAD(void, master, on_exit); }

  void on_register(const event_ptr &event, const Register &register_data) override {
    PYBIND11_OVERLOAD_PURE(void, master, on_register, event, register_data);
  }

  void on_interval_check(int64_t nanotime) override {
    PYBIND11_OVERLOAD_PURE(void, master, on_interval_check, nanotime);
  }

  int64_t acquire_trading_day() override { PYBIND11_OVERLOAD_PURE(int64_t, master, acquire_trading_day); }
};

class PyApprentice : public apprentice {
public:
  using apprentice::apprentice;

  void on_exit() override { PYBIND11_OVERLOAD_PURE(void, apprentice, on_exit); }

  void on_trading_day(const event_ptr &event, int64_t daytime) override {
    PYBIND11_OVERLOAD(void, apprentice, on_trading_day, event, daytime);
  }
};

template <typename DataType> DataType event_to_data(const event &e) { return e.data<DataType>(); }

void bind(pybind11::module &&m) {
  yijinjing::ensure_sqlite_initilize();

  m.def("thread_id", &get_thread_id);
  m.def("in_color_terminal", &in_color_terminal);
  m.def("color_print", &color_print);

  // nanosecond-time related
  m.def("now_in_nano", &time::now_in_nano);
  m.def("strftime", &time::strftime, py::arg("nanotime"), py::arg("format") = KUNGFU_TIMESTAMP_FORMAT);
  m.def("strptime", py::overload_cast<const std::string &, const std::string &>(&time::strptime), py::arg("timestr"),
        py::arg("format") = KUNGFU_TIMESTAMP_FORMAT);
  m.def("strfnow", &time::strfnow, py::arg("format") = KUNGFU_TIMESTAMP_FORMAT);

  m.def("setup_log", &kungfu::yijinjing::log::setup_log);

  m.def("hash_32", &hash_32, py::arg("key"), py::arg("length"), py::arg("seed") = KUNGFU_HASH_SEED);
  m.def("hash_str_32", &hash_str_32, py::arg("key"), py::arg("seed") = KUNGFU_HASH_SEED);
  m.def("get_page_path", &page::get_page_path);

  py::enum_<nanomsg::protocol>(m, "protocol", py::arithmetic(), "Nanomsg Protocol")
      .value("REPLY", nanomsg::protocol::REPLY)
      .value("REQUEST", nanomsg::protocol::REQUEST)
      .value("PUSH", nanomsg::protocol::PUSH)
      .value("PULL", nanomsg::protocol::PULL)
      .value("PUBLISH", nanomsg::protocol::PUBLISH)
      .value("SUBSCRIBE", nanomsg::protocol::SUBSCRIBE)
      .export_values();

  auto event_class = py::class_<event, PyEvent, std::shared_ptr<event>>(m, "event");
  event_class.def_property_readonly("gen_time", &event::gen_time)
      .def_property_readonly("trigger_time", &event::trigger_time)
      .def_property_readonly("source", &event::source)
      .def_property_readonly("dest", &event::dest)
      .def_property_readonly("msg_type", &event::msg_type)
      .def_property_readonly("data_length", &event::data_length)
      .def_property_readonly("data_as_bytes", &event::data_as_bytes)
      .def_property_readonly("data_as_string", &event::data_as_string)
      .def("to_string", &event::to_string);
  boost::hana::for_each(AllDataTypes, [&](auto pair) {
    using DataType = typename decltype(+boost::hana::second(pair))::type;
    event_class.def(boost::hana::first(pair).c_str(), &event_to_data<DataType>);
  });

  py::class_<frame, event, frame_ptr>(m, "frame")
      .def_property_readonly("frame_length", &frame::frame_length)
      .def("has_data", &frame::has_data);

  auto location_class = py::class_<location, location_ptr>(m, "location");
  location_class.def(py::init<mode, category, const std::string &, const std::string &, locator_ptr>())
      .def_readonly("mode", &location::mode)
      .def_readonly("category", &location::category)
      .def_readonly("group", &location::group)
      .def_readonly("name", &location::name)
      .def_readonly("uname", &location::uname)
      .def_readonly("uid", &location::uid)
      .def_readonly("locator", &location::locator)
      .def("__repr__", [&](location &target) { return target.uname; });
  location_class.def("to", py::overload_cast<Config &>(&location::to<Config>, py::const_));
  location_class.def("to", py::overload_cast<Register &>(&location::to<Register>, py::const_));
  location_class.def("to", py::overload_cast<Deregister &>(&location::to<Deregister>, py::const_));
  location_class.def("to", py::overload_cast<Location &>(&location::to<Location>, py::const_));

  py::class_<locator, PyLocator, locator_ptr>(m, "locator")
      .def(py::init())
      .def(py::init<const std::string &>())
      .def("has_env", &locator::has_env)
      .def("get_env", &locator::get_env)
      .def("layout_dir", &locator::layout_dir)
      .def("layout_file", &locator::layout_file)
      .def("list_page_id", &locator::list_page_id)
      .def("list_locations", &locator::list_locations, py::arg("category") = "*", py::arg("group") = "*",
           py::arg("name") = "*", py::arg("mode") = "*")
      .def("list_location_dest", &locator::list_location_dest);

  py::class_<socket, socket_ptr>(m, "socket")
      .def(py::init<protocol>(), py::arg("protocol"))
      .def("setsockopt", &socket::setsockopt_str, py::arg("level"), py::arg("option"), py::arg("value"))
      .def("setsockopt", &socket::setsockopt_int, py::arg("level"), py::arg("option"), py::arg("value"))
      .def("getsockopt", &socket::getsockopt_int, py::arg("level"), py::arg("option"))
      .def("bind", &socket::bind, py::arg("url"))
      .def("connect", &socket::connect, py::arg("url"))
      .def("shutdown", &socket::shutdown, py::arg("how") = 0)
      .def("close", &socket::close)
      .def("send", &socket::send, py::arg("msg"), py::arg("flags") = 0)
      .def("recv", &socket::recv_msg, py::arg("flags") = 0)
      .def("last_message", &socket::last_message);

  py::class_<publisher, PyPublisher, publisher_ptr>(m, "publisher")
      .def("publish", &publisher::publish)
      .def("notify", &publisher::notify);

  py::class_<observer, PyObserver, observer_ptr>(m, "observer")
      .def("wait", &observer::wait)
      .def("get_notice", &observer::get_notice);

  py::class_<reader, reader_ptr>(m, "reader")
      .def("subscribe", &reader::join)
      .def("current_frame", &reader::current_frame)
      .def("seek_to_time", &reader::seek_to_time)
      .def("data_available", &reader::data_available)
      .def("next", &reader::next)
      .def("join", &reader::join)
      .def("disjoin", &reader::disjoin);

  auto writer_class = py::class_<writer, writer_ptr>(m, "writer");
  writer_class.def(py::init<const data::location_ptr &, uint32_t, bool, publisher_ptr>())
      .def("current_frame_uid", &writer::current_frame_uid)
      .def("copy_frame", &writer::copy_frame)
      .def("mark", &writer::mark)
      .def("mark_at", &writer::mark_at);
  boost::hana::for_each(AllDataTypes, [&](auto type) {
    using DataType = typename decltype(+boost::hana::second(type))::type;
    writer_class.def("write", py::overload_cast<int64_t, const DataType &, int32_t>(&writer::write<DataType>),
                     py::arg("trigger_time"), py::arg("data"), py::arg("msg_type") = DataType::tag);
    writer_class.def("write_at", py::overload_cast<int64_t, int64_t, const DataType &>(&writer::write_at<DataType>),
                     py::arg("gen_time"), py::arg("trigger_time"), py::arg("data"));
  });

  py::class_<sink, PySink, sink_ptr>(m, "sink")
      .def(py::init())
      .def_property_readonly("publisher", &sink::get_publisher)
      .def("put", &sink::put)
      .def("close", &sink::close);

  py::class_<null_sink, sink, std::shared_ptr<null_sink>>(m, "null_sink").def(py::init<>());

  py::class_<copy_sink, sink, std::shared_ptr<copy_sink>>(m, "copy_sink")
      .def(py::init<data::locator_ptr>())
      .def("put", &copy_sink::put);

  py::class_<assemble, assemble_ptr>(m, "assemble")
      .def(py::init<const std::vector<data::locator_ptr> &, const std::string &, const std::string &,
                    const std::string &, const std::string &>(),
           py::arg("locators"), py::arg("mode") = "*", py::arg("category") = "*", py::arg("group") = "*",
           py::arg("name") = "*")
      .def("__plus__", &assemble::operator+)
      .def("__rshift__", &assemble::operator>>);

  py::class_<io_device, io_device_ptr>(m, "io_device")
      .def(py::init<location_ptr, bool, bool>(), py::arg("home"), py::arg("low_latency") = false,
           py::arg("lazy") = true)
      .def_property_readonly("publisher", &io_device::get_publisher)
      .def_property_readonly("observer", &io_device::get_observer)
      .def_property_readonly("home", &io_device::get_home)
      .def_property_readonly("live_home", &io_device::get_live_home)
      .def("is_usable", &io_device::is_usable)
      .def("setup", &io_device::setup)
      .def("open_reader", &io_device::open_reader)
      .def("open_reader_to_subscribe", &io_device::open_reader_to_subscribe)
      .def("open_writer", &io_device::open_writer)
      .def("connect_socket", &io_device::connect_socket, py::arg("location"), py::arg("protocol"),
           py::arg("timeout") = 0);

  py::class_<io_device_master, io_device, io_device_master_ptr>(m, "io_device_master")
      .def(py::init<location_ptr, bool>(), py::arg("home"), py::arg("low_latency"));

  py::class_<io_device_client, io_device, io_device_client_ptr>(m, "io_device_client")
      .def(py::init<location_ptr, bool>(), py::arg("home"), py::arg("low_latency"));

  py::class_<io_device_console, io_device, io_device_console_ptr>(m, "io_device_console")
      .def(py::init<location_ptr, uint32_t, uint32_t>(), py::arg("home"), py::arg("width"), py::arg("height"))
      .def("trace", &io_device_console::trace);

  py::class_<session_finder, std::shared_ptr<session_finder>>(m, "session_finder")
      .def(py::init<io_device_ptr>())
      .def("find_sessions", &session_finder::find_sessions, py::arg("from") = 0, py::arg("to") = INT64_MAX)
      .def("find_sessions_for", &session_finder::find_sessions_for, py::arg("source"), py::arg("from") = 0,
           py::arg("to") = INT64_MAX);

  py::class_<session_builder, session_finder, std::shared_ptr<session_builder>>(m, "session_builder")
      .def(py::init<io_device_ptr>())
      .def("rebuild_index_db", &session_builder::rebuild_index_db);

  auto profile_class = py::class_<profile, std::shared_ptr<profile>>(m, "profile");
  profile_class.def(py::init<const yijinjing::locator_ptr &>());
  boost::hana::for_each(longfist::ProfileDataTypes, [&](auto type) {
    using DataType = typename decltype(+boost::hana::second(type))::type;
    profile_class.def("set", &profile::set<DataType>);
    profile_class.def("get", &profile::get<DataType>);
    profile_class.def("get_all", &profile::get_all<DataType>);
    profile_class.def("remove", &profile::remove<DataType>);
  });

  py::class_<master, PyMaster>(m, "master")
      .def(py::init<location_ptr, bool>(), py::arg("home"), py::arg("low_latency") = false)
      .def_property_readonly("session_builder", &master::get_session_builder)
      .def_property_readonly("io_device", &master::get_io_device)
      .def_property_readonly("home", &master::get_home)
      .def_property_readonly("live", &master::is_live)
      .def("now", &master::now)
      .def("run", &master::run)
      .def("setup", &master::setup)
      .def("step", &master::step)
      .def("on_exit", &master::on_exit)
      .def("on_register", &master::on_register)
      .def("on_interval_check", &master::on_interval_check)
      .def("acquire_trading_day", &master::acquire_trading_day)
      .def("publish_trading_day", &master::publish_trading_day)
      .def("deregister_app", &master::deregister_app);

  py::class_<apprentice, PyApprentice, apprentice_ptr>(m, "apprentice")
      .def(py::init<location_ptr, bool>(), py::arg("home"), py::arg("low_latency") = false)
      .def_property_readonly("io_device", &apprentice::get_io_device)
      .def_property_readonly("home", &apprentice::get_home)
      .def_property_readonly("live", &apprentice::is_live)
      .def("set_begin_time", &apprentice::set_begin_time)
      .def("set_end_time", &apprentice::set_end_time)
      .def("on_trading_day", &apprentice::on_trading_day)
      .def("run", &apprentice::run)
      .def("setup", &apprentice::setup)
      .def("step", &apprentice::step);
}
} // namespace kungfu::yijinjing

#endif // KUNGFU_PY_YIJINJING_HPP
