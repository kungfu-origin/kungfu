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
 * JournalWriter
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * enable user to write in journal.
 * one journal writer can only write one journal,
 * and meanwhile this journal cannot be linked by other writer
 */

#include <mutex> // used by JournalSafeWriter

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/journal.h>

USING_YJJ_NAMESPACE

const string JournalWriter::FILE_PREFIX = "writer";

void JournalWriter::init(const string& dir, const string& jname)
{
    addJournal(dir, jname);
    journal = journals[0];
    seekEnd();
}

short JournalWriter::getPageNum() const
{
    return journal->getCurPageNum();
}

int64_t JournalWriter::writeStr(const string& str)
{
    return write_frame(str.c_str(), str.length() + 1, 0, MSG_TYPE_PYTHON_OBJ, 1, -1);
}

int64_t JournalWriter::writePyData(uintptr_t data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                               FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId)
{
    return write_frame((void*)data, length, source, msgType, lastFlag, requestId);
}

int64_t JournalWriter::write_frame_full(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                      FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId, FH_TYPE_NANOTM extraNano,
                                      FH_TYPE_ERR_ID errorId, const char* errorMsg)
{
    void* buffer = journal->locateFrame();
    Frame frame(buffer);
    frame.setSource(source);
    frame.setMsgType(msgType);
    frame.setLastFlag(lastFlag);
    frame.setRequestId(requestId);
    frame.setErrorData(errorId, errorMsg, data, length);
    frame.setExtraNano(extraNano);
    int64_t nano = getNanoTime();
    frame.setNano(nano);
    frame.setStatusWritten();
    journal->passFrame();
    return nano;
}

JournalWriterPtr JournalWriter::create(const string& dir, const string& jname, const string& writerName)
{
#ifdef _WINDOWS
    kungfu::log::LogConfig::setup_log(writerName);
#endif
    return JournalWriter::create(dir, jname, writerName, true);
}

JournalWriterPtr JournalWriter::create(const string& dir, const string& jname, const string& writerName, const bool client)
{
    PageProviderPtr provider;
    if (client)
    {
        provider = PageProviderPtr(new ClientPageProvider(writerName, true));
    }
    else
    {
        provider = PageProviderPtr(new LocalPageProvider(true));
    }
    return JournalWriter::create(dir, jname, provider);
}

JournalWriterPtr JournalWriter::create(const string& dir, const string& jname, PageProviderPtr provider)
{
    JournalWriterPtr jwp = JournalWriterPtr(new JournalWriter(provider));
    jwp->init(dir, jname);
    return jwp;
}

JournalWriterPtr JournalWriter::create(const string& dir, const string& jname)
{
    return JournalWriter::create(dir, jname, getDefaultName(JournalWriter::FILE_PREFIX));
}

void JournalWriter::seekEnd()
{
    journal->seekTime(TIME_TO_LAST);
    journal->locateFrame();
}

std::mutex safe_writer_mtx;

int64_t JournalSafeWriter::write_frame_full(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                         FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId, FH_TYPE_NANOTM extraNano,
                                         FH_TYPE_ERR_ID errorId, const char* errorMsg)
{
    std::lock_guard<std::mutex> lck (safe_writer_mtx);
    return this->JournalWriter::write_frame_full(data, length, source, msgType, lastFlag, requestId, extraNano, errorId, errorMsg);
}

JournalWriterPtr JournalSafeWriter::create(const string& dir, const string& jname, const string& writerName)
{
    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(writerName, true));
    JournalWriterPtr jwp = JournalWriterPtr(new JournalSafeWriter(provider));
    jwp->init(dir, jname);
    return jwp;
}