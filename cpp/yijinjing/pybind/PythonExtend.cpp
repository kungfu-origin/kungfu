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

#include "PageService.h"
#include "JournalReader.h"
#include "JournalWriter.h"
#include "JournalFinder.h"
#include "Frame.hpp"
#include "Timer.h"
#include "TypeConvert.hpp"
#include "passive.h"

#include <spdlog/details/os.h>

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

    m.def("thread_id", &spdlog::details::os::thread_id);

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
