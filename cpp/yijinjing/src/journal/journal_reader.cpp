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
 * JournalReader
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * provide read access to journals.
 * one journal reader may access multiple journals
 * and combine all these streams into one stream
 * with nano time order.
 */

#include <sstream>
#include <assert.h>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>

#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>

USING_YJJ_NAMESPACE

const string JournalReader::FILE_PREFIX = "reader";

JournalReader::JournalReader(PageProviderPtr ptr): JournalHandler(ptr)
{
    journalMap.clear();
}

size_t JournalReader::addJournal(const string& dir, const string& jname)
{
    if (journalMap.find(jname) != journalMap.end())
    {
        return journalMap[jname];
    }
    else
    {
        size_t idx = JournalHandler::addJournal(dir, jname);
        journalMap[jname] = idx;
        return idx;
    }
}

JournalReaderPtr JournalReader::create(const vector<string>& dirs, const vector<string>& jnames, int64_t startTime, const string& readerName)
{
#ifdef _WINDOWS
    kungfu::log::LogConfig::setup_log(readerName);
#endif
    std::stringstream ss;
    ss << readerName << "_R";
    string clientName = ss.str();
    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(clientName, false));
    JournalReaderPtr jrp = JournalReaderPtr(new JournalReader(provider));

    assert(dirs.size() == jnames.size());
    for (size_t i = 0; i < dirs.size(); i++)
        jrp->addJournal(dirs[i], jnames[i]);
    jrp->jumpStart(startTime);
    return jrp;
}

JournalReaderPtr JournalReader::create(const string& dir, const string& jname, int64_t startTime, const string& readerName)
{
    vector<string> dirs = {dir};
    vector<string> jnames = {jname};
    return create(dirs, jnames, startTime, readerName);
}

JournalReaderPtr JournalReader::create(const vector<string>& dirs, const vector<string>& jnames, int64_t startTime)
{
    return JournalReader::create(dirs, jnames, startTime, getDefaultName(JournalReader::FILE_PREFIX));
}

JournalReaderPtr JournalReader::create(const string& dir, const string& jname, int64_t startTime)
{
    return JournalReader::create(dir, jname, startTime, getDefaultName(JournalReader::FILE_PREFIX));
}

JournalReaderPtr JournalReader::create(const vector<string>& dirs,
                                       const vector<string>& jnames,
                                       const vector<IJournalVisitor*>& visitors,
                                       int64_t startTime,
                                       const string& readerName)
{
    JournalReaderPtr jrp = JournalReader::create(dirs, jnames, startTime, readerName);
    jrp->visitors = visitors;
    return jrp;
}

JournalReaderPtr JournalReader::createReaderWithSys(const vector<string>& dirs,
                                                    const vector<string>& jnames,
                                                    int64_t startTime,
                                                    const string& readerName)
{
    vector<string> ndirs(dirs.begin(), dirs.end());
    vector<string> njnames(jnames.begin(), jnames.end());
    ndirs.push_back(PAGED_JOURNAL_FOLDER);
    njnames.push_back(PAGED_JOURNAL_NAME);
    return create(ndirs, njnames, startTime, readerName);
}

JournalReaderPtr JournalReader::createSysReader(const string& clientName)
{
    return JournalReader::create(PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME, getNanoTime(), clientName);
}

JournalReaderPtr JournalReader::createRevisableReader(const string& readerName)
{
    std::stringstream ss;
    ss << readerName << "_SR";
    string clientName = ss.str();
    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(clientName, false, true));
    JournalReaderPtr jrp = JournalReaderPtr(new JournalReader(provider));
    jrp->addJournal(PAGED_JOURNAL_FOLDER, PAGED_JOURNAL_NAME);
    jrp->jumpStart(getNanoTime());
    return jrp;
}

void JournalReader::jumpStart(int64_t startTime)
{
    for (JournalPtr& journal: journals)
        journal->seekTime(startTime);
}

string JournalReader::getFrameName() const
{
    if (curJournal.get() == nullptr)
        return string("");
    else
        return curJournal->getShortName();
}

void JournalReader::startVisiting()
{
    FramePtr frame;
    while (true)
    {
        frame = getNextFrame();
        if (frame.get() != nullptr)
        {
            string name = getFrameName();
            for (auto visitor: visitors)
                visitor->visit(name, *frame);
        }
    }
}

bool JournalReader::addVisitor(IJournalVisitor* visitor)
{
    visitors.push_back(visitor);
    return true;
}

bool JournalReader::expireJournal(size_t idx)
{
    if (idx < journals.size())
    {
        journals[idx]->expire();
        return true;
    }
    return false;
}

bool JournalReader::seekTimeJournal(size_t idx, int64_t nano)
{
    if (idx < journals.size())
    {
        journals[idx]->seekTime(nano);
        return true;
    }
    return false;
}

bool JournalReader::expireJournalByName(const string& jname)
{
    auto iter = journalMap.find(jname);
    if (iter == journalMap.end())
        return false;
    return expireJournal(iter->second);
}

bool JournalReader::seekTimeJournalByName(const string& jname, int64_t nano)
{
    auto iter = journalMap.find(jname);
    if (iter == journalMap.end())
        return false;
    return seekTimeJournal(iter->second, nano);
}
