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

#include <spdlog/details/os.h>
#include <spdlog/details/console_globals.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/journal/page_service.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/practice/os_signal.h>

#ifdef _WINDOWS
#include <string>
#include <unordered_map>
#include <wincon.h>
class WinColor
{
public:
    const WORD BOLD = FOREGROUND_INTENSITY;
    const WORD RED = FOREGROUND_RED;
    const WORD GREEN = FOREGROUND_GREEN;
    const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
    WinColor()
    {
        colors_["trace"] = WHITE;
        colors_["debug"] = CYAN;
        colors_["info"] = GREEN;
        colors_["warn"] = YELLOW | BOLD;
        colors_["warning"] = YELLOW | BOLD;
        colors_["error"] = RED | BOLD;
        colors_["critical"] = BACKGROUND_RED | WHITE | BOLD;
        colors_["off"] = 0;
    }
    void print(std::string level, std::string log)
    {
        HANDLE out_handle_ = spdlog::details::console_stdout::handle();
        WORD color_attribs = colors_[level];
        CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
        ::GetConsoleScreenBufferInfo(out_handle_, &orig_buffer_info);
        WORD back_color = orig_buffer_info.wAttributes;
        // retrieve the current background color
        back_color &= static_cast<WORD>(~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
        // keep the background color unchanged
        ::SetConsoleTextAttribute(out_handle_, color_attribs | back_color);
        auto start = log.c_str();
        auto end = log.c_str() + log.length();
        auto size = static_cast<DWORD>(end - start);
        ::WriteFile(out_handle_, start, size, nullptr, nullptr);
        ::SetConsoleTextAttribute(out_handle_, orig_buffer_info.wAttributes); // reset to orig colors
    }
private:
    std::unordered_map<std::string, WORD> colors_;
};
WinColor color_printer;
void color_print(std::string level, std::string log)
{
    color_printer.print(level, log);
}
#else

void color_print(std::string level, std::string log)
{}

#endif

bool in_color_terminal()
{
    return spdlog::details::os::in_terminal(spdlog::details::console_stdout::stream()) && spdlog::details::os::is_color_terminal();
}

namespace py = pybind11;

using namespace kungfu::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::journal::paged;
using namespace kungfu::yijinjing::nanomsg;


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

PYBIND11_MODULE(pyyjj, m)
{
    m.def("thread_id", &spdlog::details::os::thread_id);
    m.def("in_color_terminal", &in_color_terminal);
    m.def("color_print", &color_print);

    m.def("handle_os_signals", &handle_os_signals);

    // nanosecond-time related
    m.def("now_in_nano", &time::now_in_nano);
    m.def("strftime", &time::strftime, py::arg("nanotime"), py::arg("format")=KUNGFU_DATETIME_FORMAT_DEFAULT);
    m.def("strptime", &time::strptime, py::arg("timestr"), py::arg("format")=KUNGFU_DATETIME_FORMAT_DEFAULT);
    m.def("strfnow", &time::strfnow, py::arg("format")=KUNGFU_DATETIME_FORMAT_DEFAULT);

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

    py::class_<frame, std::shared_ptr<frame>>(m, "frame")
            .def_property_readonly("gen_time", &frame::gen_time)
            .def_property_readonly("trigger_time", &frame::trigger_time)
            .def_property_readonly("source", &frame::source)
            .def_property_readonly("msg_type", &frame::msg_type);

    py::class_<page_fileinfo, std::shared_ptr<page_fileinfo>>(m, "page_fileinfo")
            .def_readonly("mode", &page_fileinfo::mode)
            .def_readonly("category", &page_fileinfo::category)
            .def_readonly("group", &page_fileinfo::group)
            .def_readonly("name", &page_fileinfo::name)
            .def_readonly("dir", &page_fileinfo::dir);

    py::class_<session, std::shared_ptr<session>>(m, "session")
            .def_readonly("fileinfo", &session::fileinfo)
            .def_readonly("start_time", &session::start_time)
            .def_readonly("start_page_id", &session::start_page_id)
            .def_readonly("end_time", &session::end_time)
            .def_readonly("end_page_id", &session::end_page_id)
            .def_readonly("closed", &session::closed)
            ;

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

    py::class_<master_service, std::shared_ptr<master_service>>(m, "master_service")
            .def("request", &master_service::request);

    py::class_<master_publisher, std::shared_ptr<master_publisher>>(m, "master_publisher")
            .def("publish", &master_publisher::publish)
            .def("poke", &master_publisher::poke);

    py::class_<master_observer, std::shared_ptr<master_observer>>(m, "master_observer")
            .def("wait_for_notice", &master_observer::wait_for_notice)
            .def("get_notice", &master_observer::get_notice);

    py::class_<master_messenger, std::shared_ptr<master_messenger>>(m, "master_messenger")
            .def_property_readonly("_service", &master_messenger::get_service)
            .def_property_readonly("_publisher", &master_messenger::get_publisher)
            .def_property_readonly("_observer", &master_messenger::get_observer);

    py::class_<aggregate_reader, std::shared_ptr<aggregate_reader>>(m, "reader")
            .def("subscribe", &aggregate_reader::subscribe)
            .def("get_sessions", &aggregate_reader::get_sessions);

    py::class_<io_device, std::shared_ptr<io_device>>(m, "io_device")
            .def(py::init<std::string, bool, bool>(), py::arg("name"), py::arg("low_latency"), py::arg("master"))
            .def_property_readonly("_messenger", &io_device::get_messenger)
            .def("open_reader_to_subscribe", &io_device::open_reader_to_subscribe)
            .def("connect_socket", &io_device::connect_socket, py::arg("mode"), py::arg("category"), py::arg("group"), py::arg("name"),
                 py::arg("protocol"), py::arg("timeout") = 0)
            .def("bind_socket", &io_device::bind_socket, py::arg("mode"), py::arg("category"), py::arg("group"), py::arg("name"), py::arg("protocol"),
                 py::arg("timeout") = 0);

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
            .def(py::init<bool>())
            .def_property_readonly("_io_device", &page_service::get_io_device)
            .def_property_readonly("_memory_msg_file", &page_service::get_memory_msg_file)
            .def_property_readonly("_memory_msg_file_size", &page_service::get_memory_msg_file_size)
            .def("process_memory_message", &page_service::process_memory_message)
            .def("get_mm_block", &page_service::get_mm_block)
            .def("release_mm_block", &page_service::release_mm_block);
}
