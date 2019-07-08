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
#include <kungfu/practice/master.h>
#include <kungfu/practice/apprentice.h>

namespace py = pybind11;

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;
using namespace kungfu::yijinjing::util;
using namespace kungfu::practice;

class PyLocator : public data::locator
{
    using data::locator::locator;

    const std::string layout_dir(data::location_ptr location, data::layout l) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::locator, layout_dir, location, l)
    }

    const std::string layout_file(data::location_ptr location, data::layout l, const std::string &name) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::locator, layout_file, location, l, name)
    }

    const std::string default_to_system_db(data::location_ptr location, const std::string &name) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::string, data::locator, default_to_system_db, location, name)
    }

    const std::vector<int> list_page_id(data::location_ptr location, uint32_t dest_id) const override
    {
        PYBIND11_OVERLOAD_PURE(const std::vector<int>, data::locator, list_page_id, location, dest_id)
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

    int32_t msg_type() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, msg_type,)
    }

    uint32_t source() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, source,)
    }

    uint32_t dest() const override
    {
        PYBIND11_OVERLOAD_PURE(int64_t, event, dest,)
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

class PyMaster : public master
{
public:
    using master::master;

    void on_notice(kungfu::yijinjing::event_ptr event) override
    {
        PYBIND11_OVERLOAD(void, master, on_notice, event);
    }

    void on_interval_check(int64_t nanotime) override
    {
        PYBIND11_OVERLOAD(void, master, on_interval_check, nanotime);
    }
};

msg::data::RequestReadFrom get_RequestReadFrom(frame_ptr f)
{
    return f->data<msg::data::RequestReadFrom>();
}

PYBIND11_MODULE(pyyjj, m)
{
    m.def("thread_id", &spdlog::details::os::thread_id);
    m.def("in_color_terminal", &in_color_terminal);
    m.def("color_print", &color_print);

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
    m.def("get_mode_name", &data::get_mode_name);

    py::enum_<data::category>(m, "category", py::arithmetic(), "Kungfu Data Category")
            .value("MD", data::category::MD)
            .value("TD", data::category::TD)
            .value("STRATEGY", data::category::STRATEGY)
            .value("SYSTEM", data::category::SYSTEM)
            .export_values();
    m.def("get_category_name", &data::get_category_name);

    py::enum_<data::layout>(m, "layout", py::arithmetic(), "Kungfu Data Layout")
            .value("JOURNAL", data::layout::JOURNAL)
            .value("SQLITE", data::layout::SQLITE)
            .value("NANOMSG", data::layout::NANOMSG)
            .value("LOG", data::layout::LOG)
            .export_values();
    m.def("get_layout_name", &data::get_layout_name);

    py::class_<event, PyEvent, std::shared_ptr<event>>(m, "event")
            .def_property_readonly("gen_time", &event::gen_time)
            .def_property_readonly("trigger_time", &event::trigger_time)
            .def_property_readonly("source", &event::source)
            .def_property_readonly("dest", &event::dest)
            .def_property_readonly("msg_type", &event::msg_type)
            .def_property_readonly("data_length", &event::data_length)
            .def_property_readonly("data_as_bytes", &event::data_as_bytes)
            .def_property_readonly("data_as_string", &event::data_as_string)
            .def("to_string", &event::to_string)
            ;

    py::class_<frame, frame_ptr>(m, "frame")
            .def_property_readonly("gen_time", &frame::gen_time)
            .def_property_readonly("trigger_time", &frame::trigger_time)
            .def_property_readonly("source", &frame::source)
            .def_property_readonly("dest", &frame::dest)
            .def_property_readonly("msg_type", &frame::msg_type)
            .def_property_readonly("frame_length", &frame::frame_length)
            .def_property_readonly("header_length", &frame::header_length)
            .def_property_readonly("data_length", &frame::data_length)
            .def_property_readonly("address", &frame::address)
            .def("data_as_bytes", &frame::data_as_bytes)
            .def("data_as_string", &frame::data_as_string)
            .def("has_data", &frame::has_data);

    py::class_<data::location, std::shared_ptr<data::location>>(m, "location")
            .def(py::init<data::mode, data::category, const std::string &, const std::string &, data::locator_ptr>())
            .def_readonly("mode", &data::location::mode)
            .def_readonly("category", &data::location::category)
            .def_readonly("group", &data::location::group)
            .def_readonly("name", &data::location::name)
            .def_readonly("uname", &data::location::uname)
            .def_readonly("uid", &data::location::uid)
            .def_readonly("locator", &data::location::locator);

    py::class_<data::locator, PyLocator, std::shared_ptr<data::locator>>(m, "locator")
            .def(py::init())
            .def("layout_dir", &data::locator::layout_dir)
            .def("layout_file", &data::locator::layout_file)
            .def("list_page_id", &data::locator::list_page_id);

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

    py::class_<reader, reader_ptr>(m, "reader")
            .def("subscribe", &reader::join)
            .def("current_frame", &reader::current_frame)
            .def("seek_to_time", &reader::seek_to_time)
            .def("data_available", &reader::data_available)
            .def("next", &reader::next)
            .def("join", &reader::join);

    py::class_<writer, writer_ptr>(m, "writer")
            .def("write_raw", &writer::write_raw);

    py::class_<io_device, io_device_ptr> io_device(m, "io_device");
    io_device.def_property_readonly("publisher", &io_device::get_publisher)
            .def_property_readonly("observer", &io_device::get_observer)
            .def_property_readonly("home", &io_device::get_home)
            .def_property_readonly("live_home", &io_device::get_live_home)
            .def("open_reader", &io_device::open_reader)
            .def("open_reader_to_subscribe", &io_device::open_reader_to_subscribe)
            .def("open_writer", &io_device::open_writer)
            .def("connect_socket", &io_device::connect_socket, py::arg("location"), py::arg("protocol"), py::arg("timeout") = 0);

    py::class_<io_device_master, io_device_master_ptr>(m, "io_device_master", io_device)
            .def(py::init<data::location_ptr, bool>());

    py::class_<io_device_client, io_device_client_ptr>(m, "io_device_client", io_device)
            .def(py::init<data::location_ptr, bool>());

    py::class_<master, PyMaster>(m, "master")
            .def(py::init<data::location_ptr, bool>(), py::arg("home"), py::arg("low_latency") = false)
            .def_property_readonly("io_device", &master::get_io_device)
            .def("run", &master::run)
            .def("on_notice", &master::on_notice)
            .def("on_interval_check", &master::on_interval_check)
            .def("deregister_app", &master::deregister_app)
            ;

    py::class_<apprentice, apprentice_ptr>(m, "apprentice")
            .def(py::init<data::location_ptr, bool>(), py::arg("home"), py::arg("low_latency") = false)
            .def_property_readonly("io_device", &apprentice::get_io_device)
            .def("set_begin_time", &apprentice::set_begin_time)
            .def("set_end_time", &apprentice::set_end_time)
            .def("run", &apprentice::run);


    py::class_<msg::data::RequestReadFrom, std::shared_ptr<msg::data::RequestReadFrom>>(m, "RequestReadFrom")
            .def_readonly("source_id", &msg::data::RequestReadFrom::source_id)
            .def_readonly("from_time", &msg::data::RequestReadFrom::from_time);
    m.def("get_RequestReadFrom", &get_RequestReadFrom);
}
