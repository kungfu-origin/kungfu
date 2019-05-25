/*****************************************************************************
 * Copyright [2017] [taurus.ai]
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
/**
 * Basic Journal Python Bindings.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Centralized Python Object & Function binding.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <spdlog/details/os.h>
#include <spdlog/details/console_globals.h>

#include <kungfu/time/timer.h>

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/nanomsg/passive.h>
#include <kungfu/yijinjing/service/page_service.h>

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
void color_print(std::string level, std::string log) {}
#endif

bool in_color_terminal()
{
    return spdlog::details::os::in_terminal(spdlog::details::console_stdout::stream()) && spdlog::details::os::is_color_terminal();
}

namespace py = pybind11;

USING_YJJ_NAMESPACE

JournalReaderPtr createReader(const vector<string>& dirs, const vector<string>& jnames, int64_t startTime, const string& readerName)
{
    return JournalReader::create(dirs, jnames, startTime, readerName);
}

JournalWriterPtr createWriter(const string& dir, const string& jname, const string& writerName)
{
    return JournalWriter::create(dir, jname, writerName);
}

vector<std::string> get_all_journal_names()
{
    JournalFinder finder;
    return finder.getAllJournalNames();
}

vector<std::string> get_available_journal_names()
{
    JournalFinder finder;
    return finder.getAvailableJournalNames();
}

vector<std::string> get_available_journal_folders()
{
    JournalFinder finder;
    return finder.getAvailableJournalFolders();
}

std::string get_journal_folder(const std::string & name)
{
    JournalFinder finder;
    return finder.getJournalFolder(name);
}

PYBIND11_MODULE(pyyjj, m)
{
    m.def("thread_id", &spdlog::details::os::thread_id);
    m.def("in_color_terminal", &in_color_terminal);
    m.def("color_print", &color_print);

    py::class_<PageService>(m, "PageService")
    .def(py::init<const std::string&>())
    .def("memory_msg_file", &PageService::get_memory_msg_file)
    .def("memory_msg_file_size", &PageService::get_memory_msg_file_size)
    .def("start", &PageService::start)
    .def("stop", &PageService::stop)
    .def("write", &PageService::write, py::arg("content"), py::arg("msg_type"), py::arg("is_last")=true, py::arg("source")=0)
    .def("process_memory_message", &PageService::process_memory_message)
    .def("register_journal", &PageService::register_journal, py::arg("clientName"))
    .def("register_client", &PageService::register_client, py::arg("clientName"), py::arg("pid"), py::arg("isWriter"))
    .def("release_page", &PageService::release_page_at, py::arg("idx"))
    ;

    py::class_<passive::emitter>(m, "emitter")
    .def(py::init<>())
    .def("poke", &passive::emitter::poke)
    .def("emit", &passive::emitter::emit)
    ;

    py::class_<passive::notice>(m, "notice")
    .def(py::init<>())
    .def("wait", &passive::notice::wait)
    .def("last_message", &passive::notice::last_message)
    ;

    // nanosecond-time related
    m.def("nano_time", &getNanoTime);
    /** two functions named of parseNano with different inputs, we needs to specify here. */
    string (*pyParseNano)(int64_t, const char*) = &parseNano;
    m.def("parse_time", &parseTime);
    m.def("parse_nano", pyParseNano);

    // create reader / writer / strategy-writer
    m.def("create_reader", &createReader);
    m.def("create_writer", &createWriter);

    // JournalReader
    py::class_<JournalReader>(m, "Reader")
    .def("add_journal", &JournalReader::addJournal, py::arg("folder"), py::arg("jname"))
    .def("expire_journal", &JournalReader::expireJournalByName, py::arg("jname"))
    .def("seek_time_by_name", &JournalReader::seekTimeJournalByName, py::arg("jname"), py::arg("nano"))
    .def("seek_time", &JournalReader::seekTimeJournal, py::arg("idx"), py::arg("nano"))
    .def("next", &JournalReader::getNextFrame)
    .def("name", &JournalReader::getFrameName);

    // JournalWriter
    py::class_<JournalWriter>(m, "Writer")
    .def("write_str", &JournalWriter::writeStr)
    .def("get_page_num", &JournalWriter::getPageNum)
    .def("write", &JournalWriter::writePyData);

    // Frame
    py::class_<Frame>(m, "Frame")
    .def_property_readonly("status", &Frame::getStatus)
    .def_property_readonly("nano", &Frame::getNano)
    .def_property_readonly("extra_nano", &Frame::getExtraNano)
    .def_property_readonly("source", &Frame::getSource)
    .def_property_readonly("msg_type", &Frame::getMsgType)
    .def_property_readonly("is_last", &Frame::getLastFlag)
    .def_property_readonly("request_id", &Frame::getRequestId)
    .def_property_readonly("error_id", &Frame::getErrorId)
    .def_property_readonly("error_msg", &Frame::getPyErrorMsg)
    .def("get_str", &Frame::getStr)
    .def("get_data", &Frame::getPyData);

    // JournalFinder
    m.def("all_journal_names", &get_all_journal_names);
    m.def("available_journal_names", &get_available_journal_names);
    m.def("available_journal_folders", &get_available_journal_folders);
    m.def("journal_folder", &get_journal_folder, py::arg("name"));

}
