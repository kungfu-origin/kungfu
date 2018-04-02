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

#include "JournalFinder.h"
#include "JournalReader.h"
#include "JournalWriter.h"
#include "StrategyUtil.h"
#include "Frame.hpp"
#include "Timer.h"
#include "PosHandler.hpp"
#include "TypeConvert.hpp"

USING_YJJ_NAMESPACE
using namespace boost::python;

JournalReaderPtr createReader(const boost::python::list& _dirs, const boost::python::list& _jnames, const string& readerName, long startTime)
{
    vector<string> dirs = py_list_to_std_vector<string>(_dirs);
    vector<string> jnames = py_list_to_std_vector<string>(_jnames);
    return JournalReader::create(dirs, jnames, startTime, readerName);
}

JournalWriterPtr createWriter(const string& dir, const string& jname, const string& writerName)
{
    return JournalWriter::create(dir, jname, writerName);
}

StrategyUtilPtr createBL(const string& strategyName)
{
    return StrategyUtil::create(strategyName);
}

PosHandlerPtr createPosHandler(short source, const string& js_str)
{
    return PosHandler::create(source, js_str);
}

PosHandlerPtr createEmptyPosHandler(short source)
{
    return PosHandler::create(source);
}

boost::python::list get_all_journal_names()
{
    JournalFinder finder;
    return std_vector_to_py_list(finder.getAllJournalNames());
}

boost::python::list get_available_journal_names()
{
    JournalFinder finder;
    return std_vector_to_py_list(finder.getAvailableJournalNames());
}

boost::python::list get_available_journal_folders()
{
    JournalFinder finder;
    return std_vector_to_py_list(finder.getAvailableJournalFolders());
}

std::string get_journal_folder(const std::string & name)
{
    JournalFinder finder;
    return finder.getJournalFolder(name);
}

BOOST_PYTHON_MODULE(libjournal)
{
    // nanosecond-time related
    def("nano", &getNanoTime);
    /** two functions named of parseNano with different inputs, we needs to specify here. */
    string (*pyParseNano)(long, const char*) = &parseNano;
    def("parse_time", &parseTime);
    def("parse_nano", pyParseNano);

    // create reader / writer / strategy-writer
    def("createReader", &createReader);
    def("createWriter", &createWriter);
    def("createBL", &createBL);
    def("createPosHandler", &createPosHandler, (arg("source"), arg("js_str")));
    def("createPosHandler0", &createEmptyPosHandler, (arg("source")));

    // JournalReader
    class_<JournalReader, boost::shared_ptr<JournalReader> >("Reader", no_init)
    .def("addJ", &JournalReader::addJournal, (arg("folder"), arg("jname")))
    .def("expireJ", &JournalReader::expireJournalByName, (arg("jname")))
    .def("restartJ", &JournalReader::seekTimeJournalByName, (arg("jname"), arg("nano")))
    .def("next", &JournalReader::getNextFrame)
    .def("name", &JournalReader::getFrameName);

    // JournalWriter
    class_<JournalWriter, boost::shared_ptr<JournalWriter> >("Writer", no_init)
    .def("write_str", &JournalWriter::writeStr)
    .def("get_page_num", &JournalWriter::getPageNum)
    .def("write", &JournalWriter::writePyData);

    // StrategyUtil
    class_<StrategyUtil, boost::shared_ptr<StrategyUtil> >("StrategyUtil", no_init)
    .def("rids", &StrategyUtil::getPyRids)
    .def("subscribe", &StrategyUtil::pySubscribe)
    .def("login_trade", &StrategyUtil::td_connect)
    .def("write_str", &StrategyUtil::writeStr)
    .def("write", &StrategyUtil::writePyData);

    // Frame
    class_<Frame, boost::shared_ptr<Frame> >("Frame", no_init)
    .def("status", &Frame::getStatus)
    .def("nano", &Frame::getNano)
    .def("extra_nano", &Frame::getExtraNano)
    .def("source", &Frame::getSource)
    .def("msg_type", &Frame::getMsgType)
    .def("is_last", &Frame::getLastFlag)
    .def("request_id", &Frame::getRequestId)
    .def("error_id", &Frame::getErrorId)
    .def("error_msg", &Frame::getPyErrorMsg)
    .def("get_str", &Frame::getStr)
    .def("get_data", &Frame::getPyData);

    // PosMap
    class_<PosHandler, boost::shared_ptr<PosHandler> >("PosHandler", no_init)
    .def("update", &PosHandler::update_py, (arg("ticker"), arg("volume"), arg("direction"), arg("trade_off"), arg("order_off")))
    .def("switch_day", &PosHandler::switch_day)
    .def("is_poisoned", &PosHandler::poisoned)
    .def("dump", &PosHandler::to_string)
    .def("get_long_tot", &PosHandler::get_long_total)
    .def("get_long_yd", &PosHandler::get_long_yestd)
    .def("get_short_tot", &PosHandler::get_short_total)
    .def("get_short_yd", &PosHandler::get_short_yestd)
    .def("get_net_tot", &PosHandler::get_net_total)
    .def("get_net_yd", &PosHandler::get_net_yestd)
    .def("get_net_fee", &PosHandler::get_net_fee)
    .def("get_net_balance", &PosHandler::get_net_balance)
    .def("get_long_fee", &PosHandler::get_long_fee)
    .def("get_long_balance", &PosHandler::get_long_balance)
    .def("get_short_fee", &PosHandler::get_short_fee)
    .def("get_short_balance", &PosHandler::get_short_balance)
    .def("get_tickers", &PosHandler::get_py_tickers)
    .def("set_pos", &PosHandler::set_pos_py, (arg("ticker"), arg("posi_direction"), arg("tot")=0, arg("yd")=0, arg("balance")=0, arg("fee")=0))
    .def("add_pos", &PosHandler::add_pos_py, (arg("ticker"), arg("posi_direction"), arg("tot")=0, arg("yd")=0, arg("balance")=0, arg("fee")=0));

    // JournalFinder
    def("get_all_journal_names", &get_all_journal_names);
    def("get_available_journal_names", &get_available_journal_names);
    def("get_available_journal_folders", &get_available_journal_folders);
    def("get_journal_folder", &get_journal_folder);
}