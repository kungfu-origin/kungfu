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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/practice/apprentice.h>

namespace py = pybind11;

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;
using namespace kungfu::yijinjing::util;
using namespace kungfu::practice;

class PyLocation : public data::location
{
    using data::location::location;

    const std::string journal_path() const override
    {
        PYBIND11_OVERLOAD_PURE(std::string, data::location, journal_path,)
    }

    const std::string socket_path() const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::location, socket_path,)
    }

    const std::string log_path() const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::location, log_path,)
    }

    const std::string make_path(const std::string& parent, const std::string& filename) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::location, make_path, parent, filename)
    }

    const std::vector<int> list_page_id(uint32_t dest_id) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::vector<int>, data::location, list_page_id, dest_id)
    }

    const data::location_ptr make_location(data::mode m, data::category c, const std::string &group, const std::string &name) const override
    {
        PYBIND11_OVERLOAD_PURE(const data::location_ptr, data::location, make_location, m, c, group, name)
    }
};

class PyEvent : public event
{
public:
    int64_t gen_time() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, gen_time,)
    }

    int64_t trigger_time() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, trigger_time,)
    }

    int16_t msg_type() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, msg_type,)
    }

    int16_t source() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, source,)
    }
};

class PyEventHandler : public event_handler
{
public:
    using event_handler::event_handler; // Inherit constructors

    const std::string &get_name() const override
    {PYBIND11_OVERLOAD_PURE(const std::string&, event_handler, get_name,) }

    void configure_event_source(event_source_ptr event_source) override
    {PYBIND11_OVERLOAD_PURE(void, event_handler, configure_event_source, event_source) }

    void handle(const event *e) override
    {PYBIND11_OVERLOAD_PURE(void, event_handler, handle, e) }

    void finish() override
    {PYBIND11_OVERLOAD(void, event_handler, finish,); }
};

class PyEventSource : public event_source
{
public:

    void subscribe(const data::location_ptr location) override
    {
        PYBIND11_OVERLOAD_PURE(void, event_source, subscribe, location)
    }

    writer_ptr get_writer(uint32_t dest_id) override
    {
        PYBIND11_OVERLOAD_PURE(writer_ptr, event_source, get_writer, dest_id)
    }

    socket_ptr get_socket_reply() override
    {
        PYBIND11_OVERLOAD_PURE(socket_ptr, event_source, get_socket_reply,)
    }
};

class PyMasterService : public master_service
{
public:
    const std::string &request(const std::string &json_message) override
    {
        PYBIND11_OVERLOAD_PURE(const std::string &, master_service, request, json_message)
    }
};

class PyPublisher : public publisher
{
public:
    int notify() override
    {
        PYBIND11_OVERLOAD_PURE(int, publisher, notify,)
    }

    int publish(const std::string &json_message) override
    {
        PYBIND11_OVERLOAD_PURE(int, publisher, publish, json_message)
    }
};

class PyObserver : public observer
{
public:
    bool wait() override
    {
        PYBIND11_OVERLOAD_PURE(bool, observer, wait,)
    }

    const std::string &get_notice() override
    {
        PYBIND11_OVERLOAD_PURE(const std::string &, observer, get_notice,)
    }
};

PYBIND11_MODULE(pyyjj, m)
{
    m.def("thread_id", &spdlog::details::os::thread_id);
    m.def("in_color_terminal", &in_color_terminal);
    m.def("color_print", &color_print);

    m.def("handle_os_signals", &os::handle_os_signals);

    // nanosecond-time related
    m.def("now_in_nano", &time::now_in_nano);
    m.def("strftime", &time::strftime, py::arg("nanotime"), py::arg("format") = KUNGFU_DATETIME_FORMAT_DEFAULT);
    m.def("strptime", &time::strptime, py::arg("timestr"), py::arg("format") = KUNGFU_DATETIME_FORMAT_DEFAULT);
    m.def("strfnow", &time::strfnow, py::arg("format") = KUNGFU_DATETIME_FORMAT_DEFAULT);

    m.def("setup_log", &kungfu::yijinjing::log::setup_log);

    m.def("hash_32", &hash_32, py::arg("key"), py::arg("length"), py::arg("seed") = KUNGFU_HASH_SEED);
    m.def("hash_str_32", &hash_str_32, py::arg("key"), py::arg("seed") = KUNGFU_HASH_SEED);
    m.def("get_page_path", &page::get_page_path);

    py::enum_<data::mode>(m, "mode", py::arithmetic(), "Kungfu Run Mode")
            .value("LIVE", data::mode::LIVE)
            .value("DATA", data::mode::DATA)
            .value("REPLAY", data::mode::REPLAY)
            .value("BACKTEST", data::mode::BACKTEST)
            .export_values();

    py::enum_<data::category>(m, "category", py::arithmetic(), "Kungfu Data Category")
            .value("MD", data::category::MD)
            .value("TD", data::category::TD)
            .value("STRATEGY", data::category::STRATEGY)
            .value("SYSTEM", data::category::SYSTEM)
            .export_values();

    py::class_<event, PyEvent, std::shared_ptr<event>>(m, "event")
            .def_property_readonly("gen_time", &event::gen_time)
            .def_property_readonly("trigger_time", &event::trigger_time)
            .def_property_readonly("source", &event::source)
            .def_property_readonly("msg_type", &event::msg_type);

    py::class_<frame, frame_ptr>(m, "frame")
            .def_property_readonly("gen_time", &frame::gen_time)
            .def_property_readonly("trigger_time", &frame::trigger_time)
            .def_property_readonly("source", &frame::source)
            .def_property_readonly("msg_type", &frame::msg_type)
            .def_property_readonly("hashcode", &frame::hashcode)
            .def_property_readonly("frame_length", &frame::frame_length)
            .def_property_readonly("header_length", &frame::header_length)
            .def_property_readonly("data_length", &frame::data_length)
            .def_property_readonly("address", &frame::address)
            .def("has_data", &frame::has_data);

    py::class_<data::location, PyLocation, std::shared_ptr<data::location>>(m, "location")
            .def(py::init<data::mode, data::category, const std::string&, const std::string&>())
            .def_readonly("mode", &data::location::mode)
            .def_readonly("category", &data::location::category)
            .def_readonly("group", &data::location::group)
            .def_readonly("name", &data::location::name)
            .def_property_readonly("hash", &data::location::hash)
            .def_property_readonly("journal_path", &data::location::journal_path)
            .def_property_readonly("socket_path", &data::location::socket_path)
            .def_property_readonly("log_path", &data::location::log_path)
            .def("make_path", &data::location::make_path)
            .def("list_page_id", &data::location::list_page_id)
            .def("make_location", &data::location::make_location)
            ;

    py::enum_<nanomsg::protocol>(m, "protocol", py::arithmetic(), "Nanomsg Protocol")
            .value("REPLY", nanomsg::protocol::REPLY)
            .value("REQUEST", nanomsg::protocol::REQUEST)
            .value("PUSH", nanomsg::protocol::PUSH)
            .value("PULL", nanomsg::protocol::PULL)
            .value("PUBLISH", nanomsg::protocol::PUBLISH)
            .value("SUBSCRIBE", nanomsg::protocol::SUBSCRIBE)
            .export_values();

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
            .def("last_messsage", &socket::last_message);

    py::class_<publisher, PyPublisher, publisher_ptr>(m, "publisher")
            .def("publish", &publisher::publish)
            .def("notify", &publisher::notify);

    py::class_<observer, PyObserver, observer_ptr>(m, "observer")
            .def("wait", &observer::wait)
            .def("get_notice", &observer::get_notice);

    py::class_<master_service, PyMasterService, master_service_ptr>(m, "master_service")
            .def("request", &master_service::request);

    py::class_<reader, reader_ptr>(m, "reader")
            .def("subscribe", &reader::subscribe)
            .def("current_frame", &reader::current_frame)
            .def("seek_to_time", &reader::seek_to_time)
            .def("data_available", &reader::data_available)
            .def("next", &reader::next);

    py::class_<writer, writer_ptr>(m, "writer")
            .def("open_session", &writer::open_session)
            .def("close_session", &writer::close_session)
            .def("write_raw", &writer::write_raw);

    py::class_<io_device, io_device_ptr> io_device(m, "io_device");
    io_device.def("open_reader", &io_device::open_reader)
            .def("open_writer", &io_device::open_writer)
            .def("connect_socket", &io_device::connect_socket, py::arg("location"), py::arg("protocol"), py::arg("timeout") = 0)
            .def("bind_socket", &io_device::bind_socket, py::arg("location"), py::arg("protocol"), py::arg("timeout") = 0);

    m.def("create_io_device", &io_device::create_io_device, py::arg("location"), py::arg("low_latency") = false);

    py::class_<io_device_client, io_device_client_ptr>(m, "io_device_client", io_device)
            .def_property_readonly("service", &io_device_client::get_service)
            .def_property_readonly("publisher", &io_device_client::get_publisher)
            .def_property_readonly("observer", &io_device_client::get_observer);

    m.def("create_io_device_client", &io_device_client::create_io_device, py::arg("location"), py::arg("low_latency") = false);

    py::class_<event_handler, PyEventHandler, event_handler_ptr>(m, "event_handler")
            .def(py::init())
            .def("get_name", &event_handler::get_name)
            .def("configure_event_source", &event_handler::configure_event_source)
            .def("handle", &event_handler::handle);

    py::class_<event_source, PyEventSource, event_source_ptr> py_event_source(m, "event_source");
    py_event_source.def("subscribe", &event_source::subscribe)
            .def_property_readonly("io_device", &event_source::get_io_device)
            .def_property_readonly("writer", &event_source::get_writer)
            .def_property_readonly("socket_reply", &event_source::get_socket_reply);

    py::class_<apprentice, std::shared_ptr<apprentice>>(m, "apprentice", py_event_source)
            .def(py::init<data::location_ptr, bool>(), py::arg("home"), py::arg("low_latency") = false)
            .def_property_readonly("io_device", &apprentice::get_io_device)
            .def("add_event_handler", &apprentice::add_event_handler)
            .def("go", &apprentice::go)
            .def("stop", &apprentice::stop);
}
