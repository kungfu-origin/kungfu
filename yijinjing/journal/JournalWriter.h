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

#ifndef YIJINJING_JOURNALWRITER_H
#define YIJINJING_JOURNALWRITER_H

#include "JournalHandler.h"
#include "FrameHeader.h"

YJJ_NAMESPACE_START

FORWARD_DECLARE_PTR(JournalWriter)
/**
 * Journal Writer
 */
class JournalWriter : public JournalHandler
{
protected:
    /** the journal will write in */
    JournalPtr journal;
    /** private constructor */
    JournalWriter(PageProviderPtr ptr): JournalHandler(ptr) {}

public:
    /** init journal */
    void init(const string& dir, const string& jname);
    /** get current page number */
    short getPageNum() const;
    /* seek to the end of the journal
     * journal can only be appended in the back,
     * no modification of existing frames is allowed. */
    void seekEnd();
    /** to write a string into journal */
    long writeStr(const string& str);
    /** same as write_frame, but address field is optimized to fit python binding */
    long writePyData(uintptr_t data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                      FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId);

    /** write a frame with full information */
    virtual long write_frame_full(const void* data, FH_TYPE_LENGTH length,
                                  FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                  FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                                  FH_TYPE_NANOTM extraNano, FH_TYPE_ERR_ID errorId,
                                  const char* errorMsg);
    /** write a basic frame */
    inline long write_frame(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                             FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, 0, 0, nullptr);
    }
    /** write a frame with extra nano */
    inline long write_frame_extra(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                                   FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag,
                                   FH_TYPE_REQ_ID requestId, FH_TYPE_NANOTM extraNano)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, extraNano, 0, nullptr);
    }
    /** write a error frame */
    inline long write_error_frame(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                            FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                            FH_TYPE_ERR_ID errorId, const char* errorMsg)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, 0, errorId, errorMsg);
    }
public:
    // creators
    static JournalWriterPtr create(const string& dir, const string& jname, const string& writerName);
    static JournalWriterPtr create(const string& dir, const string& jname, PageProviderPtr ptr);
    static JournalWriterPtr create(const string& dir, const string& jname);

public:
    static const string PREFIX;
};


class JournalSafeWriter: public JournalWriter
{
protected:
    JournalSafeWriter(PageProviderPtr ptr): JournalWriter(ptr) {}

public:
    /** write a frame with full information */
    virtual long write_frame_full(const void* data, FH_TYPE_LENGTH length,
                                  FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                  FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                                  FH_TYPE_NANOTM extraNano, FH_TYPE_ERR_ID errorId,
                                  const char* errorMsg);

    // create a thread safe writer (with mutex in write_frame)
    static JournalWriterPtr create(const string& dir, const string& jname, const string& writerName);
};

YJJ_NAMESPACE_END
#endif //YIJINJING_JOURNALWRITER_H
