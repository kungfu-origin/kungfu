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
#include <kungfu/yijinjing/journal/page_service.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/practice/apprentice.h>

namespace py = pybind11;

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::journal::paged;
using namespace kungfu::yijinjing::nanomsg;
using namespace kungfu::yijinjing::util;
using namespace kungfu::practice;

class PyEvent : public event
{
public:
    int64_t gen_time() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, gen_time, )
    }
    int64_t trigger_time() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, trigger_time, )
    }
    int16_t msg_type() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, msg_type, )
    }
    int16_t source() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, source, )
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

    void handle(const event &e) override
    {PYBIND11_OVERLOAD_PURE(void, event_handler, handle, e) }

    void finish() override
    {PYBIND11_OVERLOAD(void, event_handler, finish,); }
};

class PyEventlet : public event_source
{
public:
    void setup_output(data::mode m, data::category c, const std::string &group, const std::string &name) override
    {
        PYBIND11_OVERLOAD_PURE(void, event_source, setup_output, m, c, group, name)
    }

    void subscribe(data::mode m, data::category c, const std::string &group, const std::string &name) override
    {
        PYBIND11_OVERLOAD_PURE(void, event_source, subscribe, m, c, group, name)
    }

    writer_ptr get_writer() override
    {
        PYBIND11_OVERLOAD_PURE(writer_ptr, event_source, get_writer,)
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

    py::class_<frame, std::shared_ptr<frame>>(m, "frame")
            .def_property_readonly("gen_time", &frame::gen_time)
            .def_property_readonly("trigger_time", &frame::trigger_time)
            .def_property_readonly("source", &frame::source)
            .def_property_readonly("msg_type", &frame::msg_type);

    py::class_<data::location, std::shared_ptr<data::location>>(m, "location")
            .def_readonly("mode", &data::location::mode)
            .def_readonly("category", &data::location::category)
            .def_readonly("group", &data::location::group)
            .def_readonly("name", &data::location::name);

    py::class_<data::session, std::shared_ptr<data::session>>(m, "session")
            .def_readonly("location", &data::session::location)
            .def_readonly("start_time", &data::session::start_time)
            .def_readonly("start_page_id", &data::session::start_page_id)
            .def_readonly("end_time", &data::session::end_time)
            .def_readonly("end_page_id", &data::session::end_page_id)
            .def_readonly("closed", &data::session::closed);

    py::enum_<nanomsg::protocol>(m, "protocol", py::arithmetic(), "Nanomsg Protocol")
            .value("REPLY", nanomsg::protocol::REPLY)
            .value("REQUEST", nanomsg::protocol::REQUEST)
            .value("PUSH", nanomsg::protocol::PUSH)
            .value("PULL", nanomsg::protocol::PULL)
            .value("PUBLISH", nanomsg::protocol::PUBLISH)
            .value("SUBSCRIBE", nanomsg::protocol::SUBSCRIBE)
            .export_values();

    py::class_<socket, std::shared_ptr<socket>>(m, "socket")
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

    py::class_<publisher, std::shared_ptr<publisher>>(m, "publisher")
            .def("notify", &publisher::notify)
            .def("publish", &publisher::publish);

    py::class_<master_service, PyMasterService, std::shared_ptr<master_service>>(m, "master_service")
            .def("request", &master_service::request);

//    py::class_<publisher, std::shared_ptr<publisher>>(m, "publisher")
//            .def("publish", &publisher::publish)
//            .def("notify", &publisher::notify);
//
//    py::class_<observer, std::shared_ptr<observer>>(m, "observer")
//            .def("wait", &observer::wait)
//            .def("get_notice", &observer::get_notice);

    py::class_<aggregate_reader, std::shared_ptr<aggregate_reader>>(m, "reader")
            .def("subscribe", &aggregate_reader::subscribe)
            .def("get_sessions", &aggregate_reader::get_sessions);

    py::class_<io_device, std::shared_ptr<io_device>> io_device(m, "io_device");
    io_device.def("open_reader_to_subscribe", &io_device::open_reader_to_subscribe)
            .def("connect_socket", &io_device::connect_socket, py::arg("mode"), py::arg("category"), py::arg("group"), py::arg("name"),
                 py::arg("protocol"), py::arg("timeout") = 0)
            .def("bind_socket", &io_device::bind_socket, py::arg("mode"), py::arg("category"), py::arg("group"), py::arg("name"), py::arg("protocol"),
                 py::arg("timeout") = 0);

    m.def("create_io_device", &io_device::create_io_device, py::arg("name"), py::arg("low_latency"));

    py::class_<io_device_client, std::shared_ptr<io_device_client>>(m, "io_device_client", io_device)
            .def_property_readonly("_service", &io_device_client::get_service)
            .def_property_readonly("_publisher", &io_device_client::get_publisher);

    m.def("create_io_device_client", &io_device_client::create_io_device, py::arg("name"), py::arg("low_latency"));

    py::class_<event_handler, PyEventHandler, std::shared_ptr<event_handler>>(m, "event_handler")
            .def(py::init())
            .def("get_name", &event_handler::get_name)
            .def("configure_event_source", &event_handler::configure_event_source)
            .def("handle", &event_handler::handle);

    py::class_<event_source, PyEventlet, std::shared_ptr<event_source>> py_event_source(m, "event_source");
    py_event_source.def("setup_output", &event_source::setup_output)
            .def("subscribe", &event_source::subscribe)
            .def_property_readonly("_writer", &event_source::get_writer)
            .def_property_readonly("_socket_reply", &event_source::get_socket_reply);

    py::class_<apprentice, std::shared_ptr<apprentice>>(m, "apprentice", py_event_source)
            .def(py::init<std::string, bool>(), py::arg("name"), py::arg("low_latency") = false)
            .def_property_readonly("_io_device", &apprentice::get_io_device)
            .def("add_event_handler", &apprentice::add_event_handler)
            .def("go", &apprentice::go)
            .def("stop", &apprentice::stop);

    py::class_<page_service>(m, "page_service")
            .def(py::init<io_device_ptr>())
            .def_property_readonly("_io_device", &page_service::get_io_device)
            .def_property_readonly("_memory_msg_file", &page_service::get_memory_msg_file)
            .def_property_readonly("_memory_msg_file_size", &page_service::get_memory_msg_file_size)
            .def("process_memory_message", &page_service::process_memory_message)
            .def("get_mm_block", &page_service::get_mm_block)
            .def("release_mm_block", &page_service::release_mm_block);
}
