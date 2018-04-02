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
 * Tool for viewing / monitoring journal
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 */

#include "Timer.h"
#include "Hash.hpp"
#include "JournalReader.h"
#include "JournalFinder.h"
#include "Frame.hpp"
#include "Page.h"
#include "PageUtil.h"
#include "PageHeader.h"
#include "longfist/LFPrintUtils.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <algorithm>

using namespace boost::program_options;
USING_YJJ_NAMESPACE

/**
 * entrance for journal tool
 */
int main(int argc, const char* argv[])
{
    options_description desc{"Options"};
    string format("%Y%m%d-%H:%M:%S");
    desc.add_options()
            ("help,h", "Help screen")
            ("name,n", value<string>(), "Journal Name")
            ("page,p", "Just Page Header")
            ("verify,v", "Verify hash code")
            ("keep,k", "keep listening")
            ("time,t", "time visualized")
            ("detail,d", "data details")
            ("current,c", "start from current")
            ("length,l", value<int>()->default_value(20), "Char num of frame data to print")
            ("msgtype,m", value<string>()->default_value(""), "Message type printed, -eg: -m 10,11")
            ("rmsgtype,r", value<string>()->default_value(""), "Message type not printed, -eg: -r 10,11")
            ("start_time,s", value<string>()->default_value("20000101-13:30:00"), "start time (%Y%m%d-%H:%M:%S)")
            ("end_time,e", value<string>()->default_value("20200101-00:00:00"), "end time (%Y%m%d-%H:%M:%S)");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    JournalFinder finder;

    if (vm.count("help"))
    {
        std::cout << desc << '\n';
        std::cout << "----frame tags----" << std::endl;
        std::cout << " (st): status" << std::endl;
        std::cout << " (so): source" << std::endl;
        std::cout << " (na): nano time" << std::endl;
        std::cout << " (en): extra nano time" << std::endl;
        std::cout << " (vn): visualized nano (-t only)" << std::endl;
        std::cout << " (fl): frame length" << std::endl;
        std::cout << " (dl): data length" << std::endl;
        std::cout << " (hl): header length" << std::endl;
        std::cout << " (hs): hash code" << std::endl;
        std::cout << " (mt): msg type" << std::endl;
        std::cout << " (lf): last flag" << std::endl;
        std::cout << " (id): request id" << std::endl;
        std::cout << " (er): error id" << std::endl;
        std::cout << " (em): error message" << std::endl;
        std::cout << " (cn): data content" << std::endl;
        std::cout << std::endl;
        std::cout << "----page tags----" << std::endl;
        std::cout << " (ST): page status" << std::endl;
        std::cout << " (JN): journal name" << std::endl;
        std::cout << " (PN): page number" << std::endl;
        std::cout << " (FN): total number of frame in this page" << std::endl;
        std::cout << " (LP): last position" << std::endl;
        std::cout << " (FV): frame version" << std::endl;
        std::cout << " (SN): start nano" << std::endl;
        std::cout << " (CN): close nano" << std::endl;
        return 0;
    }
    string start_time = vm.count("current") ? parseNano(getNanoTime(), format.c_str()) : vm["start_time"].as<string>();
    if (vm.count("start_time"))
    {
        std::cout << "StartTime:\t" << start_time << std::endl;
    }
    if (vm.count("end_time"))
    {
        std::cout << "EndTime:\t" << vm["end_time"].as<string>() << std::endl;
    }

    if (vm.count("name"))
    {
        std::cout << "ShortName:\t" << vm["name"].as<string>() << std::endl;
    }
    else
    {
        vector<std::string> available_journal_names = finder.getAvailableJournalNames();
        std::cout << "usage: kungfu journal -n [--name] journal_name <options>" << std::endl;
        std::cout << "available journal names:" << std::endl;
        for (vector<std::string>::iterator it = available_journal_names.begin(); it != available_journal_names.end(); it++) {
            std::cout << '\t' << *it << std::endl;
        }
        return -1;
    }

    string jname = vm["name"].as<string>();
    string folder = finder.getJournalFolder(jname);

    if (vm.count("page"))
    {
        vector<short> pageNums = PageUtil::GetPageNums(folder, jname);

        for (size_t idx = 0; idx < pageNums.size(); idx++)
        {
            PageHeader header = PageUtil::GetPageHeader(folder, jname, pageNums[idx]);
            std::cout << "[" << pageNums[idx] << "]"
                      << " (ST)" << (short)header.status
                      << " (JN)" << header.journal_name
                      << " (PN)" << header.page_num
                      << " (FN)" << header.frame_num
                      << " (LP)" << header.last_pos
                      << " (FV)" << header.frame_version
                      << " (SN)" << header.start_nano << "[" << parseNano(header.start_nano, format.c_str()) << "]"
                      << " (CN)" << header.close_nano << "[" << parseNano(header.close_nano, format.c_str()) << "]";
            std::cout << std::endl;
        }
    }
    else
    {
        bool toVerify = vm.count("verify");
        bool toTimeVisual = vm.count("time");
        bool needDetail = vm.count("detail");
        long sn = parseTime(start_time.c_str(), format.c_str());
        long en = parseTime(vm["end_time"].as<string>().c_str(), format.c_str());
        int dataLengthToPrint = vm["length"].as<int>();

        map<short, bool> doMsgTypes, nonMsgTypes;
        string doMtStr = vm["msgtype"].as<string>();
        bool mtDoList = doMtStr.length() > 0;
        if (mtDoList)
        {
            std::stringstream ss(doMtStr);
            int i;
            while (ss >> i)
            {
                doMsgTypes[i] = true;
                if (ss.peek() == ',')
                    ss.ignore();
            }
            std::cout << "MsgType to Print:";
            for (auto& item: doMsgTypes)
            {
                std::cout << " " << item.first;
            }
            std::cout << std::endl;
        }
        string nonMtStr = vm["rmsgtype"].as<string>();
        bool mtNonList = nonMtStr.length() > 0;
        if (mtNonList)
        {
            std::stringstream ss(nonMtStr);
            int i;
            while (ss >> i)
            {
                nonMsgTypes[i] = true;
                if (ss.peek() == ',')
                {
                    ss.ignore();
                }
            }
            std::cout << "MsgType to Skip:";
            for (auto& item: nonMsgTypes)
            {
                std::cout << " " << item.first;
            }
            std::cout << std::endl;
        }
        JournalReaderPtr reader = JournalReader::create(folder, jname, sn, "JournalPrinter");
        int i = 0;
        do
        {
            FramePtr frame = reader->getNextFrame();
            while (frame.get() != nullptr && en > frame->getNano())
            {
                short msgType = frame->getMsgType();
                bool toPrint = true;
                if ((mtDoList && doMsgTypes.find(msgType) == doMsgTypes.end())
                    || (mtNonList && nonMsgTypes.find(msgType) != nonMsgTypes.end()))
                {
                    toPrint = false;
                }
                if (toPrint)
                {
                    std::cout << "[" << i++ << "]"
                              << " (st)" << (short)frame->getStatus()
                              << " (so)" << frame->getSource()
                              << " (na)" << frame->getNano()
                              << " (en)" << frame->getExtraNano();
                    if (toTimeVisual)
                    {
                        std::cout << " (vn)" << parseNano(frame->getNano(), format.c_str());
                    }
                    std::cout << " (fl)" << frame->getFrameLength()
                              << " (dl)" << frame->getDataLength()
                              << " (hl)" << frame->getHeaderLength()
                              << " (hs)" << frame->getHashCode()
                              << " (mt)" << frame->getMsgType()
                              << " (lf)" << (short)frame->getLastFlag()
                              << " (id)" << frame->getRequestId()
                              << " (er)" << frame->getErrorId();
                    if (frame->getErrorMsg() != nullptr)
                    {
                        std::cout << " (em)" << frame->getErrorMsg();
                    }
                    if (dataLengthToPrint > 0)
                    {
                        std::cout << " (cn)" << string((char*)frame->getData(), std::min(dataLengthToPrint, frame->getDataLength()));
                    }
                    if (toVerify)
                    {
                        FH_TYPE_HASHNM hash = MurmurHash2(frame->getData(), frame->getDataLength(), HASH_SEED);
                        if (hash != frame->getHashCode())
                        {
                            std::cerr << std::endl << std::endl
                                      << "Hash Code mismatch: "
                                      << "[frame] " << frame->getHashCode()
                                      << "  [actual] " << hash << std::endl;
                            return -1;
                        }
                        else
                        {
                            std::cout << " (*)";
                        }
                    }
                    std::cout << std::endl;
                    if (needDetail)
                    {
                        printData(frame->getData(), msgType);
                    }
                }
                frame = reader->getNextFrame();
            }
        }
        while (vm.count("keep"));
    }
    return 0;
}